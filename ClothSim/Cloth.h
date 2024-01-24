#pragma once
#include "Mesh.h"

enum DrawMode
{
	fill = 0,
	wireframe = 1
};

class Cloth
{
private:
	float m_width;
	float m_depth;
	GLuint m_widthDivisions;
	GLuint m_depthDivisions;
	ShaderProgram m_shader;
	ShaderProgram m_simShader;
	int m_iterations;
	int m_readID = 0;
	int m_writeID = 1;
	glm::mat4 m_modelMat = glm::translate(glm::vec3(0.0f));
	glm::vec3 m_color = glm::vec3(0.9f);

	std::vector<glm::vec4> m_initPos;
	std::vector<glm::vec2> m_texUV;
	std::vector<glm::vec3> m_normal;
	std::vector<GLuint> m_indices;
	std::vector<ClothVertex> m_vertices;

	VAO<glm::vec4> m_vaoRender[2];
	VAO<ClothVertex> m_vaoSim[2];
	VAO<ClothVertex> m_vaoDraw[2];
	VAO<glm::vec4> m_vaoUpdate[2];

	VBO<glm::vec4> m_vboCrnt[2];
	VBO<glm::vec4> m_vboPrev[2];
	VBO<glm::vec3> m_vboNormal;
	VBO<ClothVertex> m_vbo[2];
	VBO<glm::vec2> m_vboTexUV;
	
	EBO m_ebo;

	GLuint m_crntPosTex[2];
	GLuint m_prevPosTex[2];

	Texture m_texture;

	GLuint m_tf;

	void InitMesh();
	void InitVertices();
	void InitIndices();
	
public:
	Cloth(float width, float depth, GLuint widthDivisions, GLuint depthDivisions, int iterations);
	void SetModel(glm::mat4 model) { m_modelMat = model; }
	void SetColor(glm::vec3 color) { m_color = color; }
	void AttachShader(const char* vertShader, const char* fragShader);
	void AttachShader(const char* vertShader);
	void Draw(Camera& camera, DrawMode mode);
	void DrawShadow(ShaderProgram& shader, Camera& camera);
	void SetTexture(const char* image, const char* texType, GLuint slot)
	{
		m_texture = Texture(image, texType, slot);
	}
	void SetIterations(int iterations)
	{
		m_iterations = iterations;
	}
	void SetVAOs()
	{
		glGenTextures(2, m_crntPosTex);
		glGenTextures(2, m_prevPosTex);

		// set vbo
		for (int i = 0; i < 2; i++)
		{
			m_vboCrnt[i] = VBO<glm::vec4>(m_initPos, GL_DYNAMIC_COPY);
			m_vboPrev[i] = VBO<glm::vec4>(m_initPos, GL_DYNAMIC_COPY);
			m_vbo[i] = VBO<ClothVertex>(m_vertices, GL_DYNAMIC_COPY);
		}
		m_vboNormal = VBO<glm::vec3>(m_normal, GL_DYNAMIC_COPY);
		m_vboTexUV = VBO<glm::vec2>(m_texUV, GL_STATIC_DRAW);
		// set update vao
		for (int i = 0; i < 2; i++)
		{
			m_vaoUpdate[i].Bind();
			m_vaoUpdate[i].linkAttrib(m_vboCrnt[i], 0, 4, GL_FLOAT, 0, 0);
			m_vaoUpdate[i].linkAttrib(m_vboPrev[i], 1, 4, GL_FLOAT, 0, 0);
			m_vaoUpdate[i].Unbind();

			m_vaoSim[i].Bind();
			m_vaoSim[i].linkAttrib(m_vbo[i], 0, 4, GL_FLOAT, sizeof(ClothVertex), (void*)(0 * sizeof(float)));
			m_vaoSim[i].linkAttrib(m_vbo[i], 1, 4, GL_FLOAT, sizeof(ClothVertex), (void*)(4 * sizeof(float)));
			m_vaoSim[i].Unbind();
		}

		// set render vao
		for (int i = 0; i < 2; i++)
		{
			m_vaoRender[i].Bind();
			m_vaoRender[i].linkAttrib(m_vboCrnt[i], 0, 4, GL_FLOAT, 0, 0);
			m_vaoRender[i].linkAttrib(m_vboNormal, 1, 3, GL_FLOAT, 0, 0);
			m_vaoRender[i].linkAttrib(m_vboTexUV, 2, 2, GL_FLOAT, 0, 0);
			m_ebo = EBO(m_indices);
			m_ebo.Bind();
			m_vaoRender[i].Unbind();
		}
		for (int i = 0; i < 2; i++)
		{
			m_vaoDraw[i].Bind();
			m_vaoDraw[i].linkAttrib(m_vbo[i], 0, 4, GL_FLOAT, sizeof(ClothVertex), (void*)(0 * sizeof(float))); // position
			m_vaoDraw[i].linkAttrib(m_vbo[i], 1, 3, GL_FLOAT, sizeof(ClothVertex), (void*)(8 * sizeof(float))); // normal
			m_vaoDraw[i].linkAttrib(m_vbo[i], 2, 2, GL_FLOAT, sizeof(ClothVertex), (void*)(11 * sizeof(float))); // texUV
			m_ebo = EBO(m_indices);
			m_ebo.Bind();
			m_vaoDraw[i].Unbind();
		}
	}
	void GenTF(const GLchar* varyingNames[], GLuint numOfVaryings)
	{
		glGenTransformFeedbacks(1, &m_tf);
		glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, m_tf);
		glTransformFeedbackVaryings(m_simShader.id(), numOfVaryings, varyingNames, GL_SEPARATE_ATTRIBS);
		glLinkProgram(m_simShader.id());
		GLint linked;
		glGetProgramiv(m_simShader.id(), GL_LINK_STATUS, &linked);
		if (linked != 1) {
			std::cout << "linking failed" << std::endl;
			printProgramLog(m_simShader.id());
		}
	}
	void SetSimUniforms()
	{
		m_simShader.Activate();
		m_simShader.setInt("crntPosTex", 0);
		m_simShader.setInt("prevPosTex", 1);

		m_simShader.setInt("texSizeX", m_widthDivisions + 1);
		m_simShader.setInt("texSizeZ", m_depthDivisions + 1);
		m_simShader.setVec2("invClothSize",
			glm::vec2(m_width / (float)m_widthDivisions, m_depth / (float)m_depthDivisions));
		m_simShader.setInt("iterations", m_iterations);
	}
	void Simulate()
	{
		for (int i = 0; i < m_iterations; i++)
		{
			m_simShader.Activate();
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_BUFFER, m_crntPosTex[m_writeID]);
			glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, m_vboCrnt[m_writeID].id());
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_BUFFER, m_prevPosTex[m_writeID]);
			glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, m_vboPrev[m_writeID].id());

			m_vaoUpdate[m_writeID].Bind();
			glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, m_vboCrnt[m_readID].id());
			glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 1, m_vboPrev[m_readID].id());
			glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 2, m_vboNormal.id());

			glEnable(GL_RASTERIZER_DISCARD);
			glBeginTransformFeedback(GL_POINTS);
			GLuint numPoints = (m_widthDivisions + 1) * (m_depthDivisions + 1);
			glDrawArrays(GL_POINTS, 0, numPoints);
			glEndTransformFeedback();
			glFlush();
			int tmp = m_readID;
			m_readID = m_writeID;
			m_writeID = tmp;
			glDisable(GL_RASTERIZER_DISCARD);
		}
	}
};

