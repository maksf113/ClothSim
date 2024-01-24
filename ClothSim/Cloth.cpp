#include "Cloth.h"

Cloth::Cloth(float width, float depth, GLuint widthDivisions, GLuint depthDivisions, int iterations)
{
	m_width = width;
	m_depth = depth;
	m_widthDivisions = widthDivisions;
	m_depthDivisions = depthDivisions;
	m_iterations = iterations;
	InitMesh();
	InitIndices();
	SetVAOs();
	//Mesh::CreateGLState(m_vertices, m_indices);
}
void Cloth::InitMesh()
{
	InitVertices();
	InitIndices();
}

void Cloth::InitVertices()
{
	//(i, j) = (0, 0) -> (x, z) = (0, 0)
	m_initPos.resize((m_widthDivisions + 1) * (m_depthDivisions + 1));
	m_texUV.resize((m_widthDivisions + 1) * (m_depthDivisions + 1));
	m_normal.resize((m_widthDivisions + 1) * (m_depthDivisions + 1));
	m_vertices.resize((m_widthDivisions + 1) * (m_depthDivisions + 1));
	GLuint index;
	float delta_x = m_width / m_widthDivisions;
	float delta_z = m_depth / m_depthDivisions;
	for(int j = 0; j < m_depthDivisions + 1; j++)
		for (int i = 0; i < m_widthDivisions + 1; i++)
		{
			index = i + j * (m_widthDivisions + 1);
			if (index >= m_initPos.size())
				assert(0);
			m_initPos[index] = glm::vec4(float(i) * delta_x, 0.0f, float(j) * delta_z, 0.001f);
			m_texUV[index] = glm::vec2(float(i) / m_widthDivisions, float(j) / m_depthDivisions);
			m_normal[index] = glm::vec3(0.0f, 1.0f, 0.0f);
			m_vertices[index].position = glm::vec4(float(i) * delta_x, 0.0f, float(j) * delta_z, 0.001f);
			m_vertices[index].previousPosition = glm::vec4(float(i) * delta_x, 0.0f, float(j) * delta_z, 0.001f);
			m_vertices[index].texUV = glm::vec2(float(i) / m_widthDivisions, float(j) / m_depthDivisions);
		}	
}

void Cloth::InitIndices()
{
	int numQuads = m_widthDivisions * m_depthDivisions;
	m_indices.resize(numQuads * 6);
	GLuint index = 0;
	for (int j = 0; j < m_depthDivisions; j++)
		for (int i = 0; i < m_widthDivisions; i++)
		{
			GLuint bottomLeft = j * (m_widthDivisions + 1) + i;
			GLuint bottomRight = j * (m_widthDivisions + 1) + i + 1;
			GLuint topLeft = (j + 1) * (m_widthDivisions + 1) + i;
			GLuint topRight = (j + 1) * (m_widthDivisions + 1) + i + 1;

			m_indices[index++] = bottomLeft;
			m_indices[index++] = topLeft;
			m_indices[index++] = bottomRight;

			m_indices[index++] = bottomRight;
			m_indices[index++] = topLeft;
			m_indices[index++] = topRight;
		}
}
void Cloth::AttachShader(const char* vertShader, const char* fragShader)
{
	m_shader = ShaderProgram(vertShader, fragShader);
}
void Cloth::AttachShader(const char* vertShader)
{
	m_simShader = ShaderProgram(vertShader);
}

void Cloth::Draw(Camera& camera, DrawMode mode)
{
	//glBindVertexArray(m_vaoRender[i]);
	m_vaoRender[m_readID].Bind();
	m_shader.Activate();
	m_shader.setMat4("camMatrix", camera.GetMatrix());
	m_shader.setMat4("model", m_modelMat);
	m_shader.setVec3("color", m_color);
	m_shader.setVec3("camPos", camera.GetPosition());
	m_shader.setInt("tex", 0);
	m_texture.Bind();

	if (mode == wireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, 0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void Cloth::DrawShadow(ShaderProgram& shader, Camera& camera)
{
	m_vaoRender[m_readID].Bind();
	glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, 0);
}