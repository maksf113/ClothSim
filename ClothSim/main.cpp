#include "Model.h"
#include "FBO.h"
#include "Floor.h"
#include "Cloth.h"

#define numPointLights 0

GLuint width = 1920 * 1;
GLuint height = 1080 * 1;
float prevTime = 0.0f;
float crntTime = 0.0f;
float dt = 0.0f;

int main(void) 
{
	// Initialize GLFW
	if (!glfwInit()) { exit(EXIT_FAILURE); }
	// Tell GLFW what version of OpenGL we are using 
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 4);
	glEnable(GL_MULTISAMPLE);
	// Create a GLFWwindow object
	GLFWwindow* window = glfwCreateWindow(width, height, "Chapter 2 - program1", NULL, NULL);
	// Introduce the window into the current context
	glfwMakeContextCurrent(window);
	// Specify the viewport of OpenGL in the Window

	glViewport(0, 0, width, height);
	if (glewInit() != GLEW_OK) { exit(EXIT_FAILURE); }
	glfwSwapInterval(1);

	Floor floor(10.0f, "planks.png", "planksSpec.png");
	//Model model("cat/cat.obj");
	ShaderProgram shader("default.vert", "default.frag");

	ShaderProgram dirShadowShader("directionalShadowMap.vert", "directionalShadowMap.frag");

	glm::vec3 floorPos = glm::vec3(0.0f, -1.0f, 0.0f);
	glm::mat4 floorModel = glm::mat4(1.0f);
	floorModel = glm::translate(floorModel, floorPos);
	
	 
	Camera camera(width, height, glm::vec3(1.4f, 2.0f, -4.0f));


	// SHADOW

	unsigned int shadowWidth = 1024 * 2, shadowHeight = 1024 * 2;

	FBO shadowFBO(shadowWidth, shadowHeight);
	GLuint shadowTexUnit = 2;
	shadowFBO.GenTexture();
	shadowFBO.AttachDepth();
	shadowFBO.DisableColor();

	glm::mat4 orthogonalProjection = glm::ortho(-7.0f, 7.0f, -7.0f, 7.0f, 0.5f, 8.0f);
	glm::mat4 lightView = glm::lookAt(4.0f * glm::vec3(-0.2f, 1.0f, -0.3f),
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 dirLightVP = orthogonalProjection * lightView;

	shadowFBO.Bind();
	shadowFBO.Viewport();
	glEnable(GL_DEPTH_TEST);
	glClear(GL_DEPTH_BUFFER_BIT);
	glDepthFunc(GL_LESS);

	dirShadowShader.Activate();
	dirShadowShader.setMat4("lightVP", dirLightVP);

	// CLOTH
	float clothWidth = 3.0f;
	float clothDepth = 3.0f;
	unsigned int clothWidthDiv = 64;
	unsigned int clothDepthDiv = 64;
	int iterations = 20;
	Cloth cloth(clothWidth, clothDepth, clothWidthDiv, clothDepthDiv, iterations);
	glm::mat4 clothModel = glm::translate(glm::vec3(0.0f, 3.0f, 0.0f));
	cloth.SetModel(clothModel);
	
	cloth.AttachShader("cloth.vert", "cloth.frag");
	cloth.AttachShader("clothSim.vert");
	GLuint numOfVaryings = 3;
	const char* varyingNames[] = { "outCrntPos", "outPrevPos", "outNormal" };
	
	cloth.GenTF(varyingNames, numOfVaryings);
	GLclearError();
	cloth.SetSimUniforms();
	GLcheckError();
	int readID = 0, writeID = 1;
	cloth.SetTexture("WG.jpg", "diffuse0", 0);
	// MAIN LOOP
	int counter = 0;
	while (!glfwWindowShouldClose(window)) 
	{
		crntTime = glfwGetTime();
		dt = crntTime - prevTime;
		counter++;
		if (dt >= 1.0 / 30.0)
		{ 
			std::string FPS = std::to_string(1.0 / dt * counter);
			std::string ms = std::to_string(dt / counter * 1000);
			std::string newTitle = "Project 8 - " + FPS + "FPS / " + ms + "ms";
			//glfwSetWindowTitle(window, newTitle.c_str());
			counter = 0;
			prevTime = crntTime;
		}

		// SHADOW
		shadowFBO.Bind();
		shadowFBO.Viewport();
		glEnable(GL_DEPTH_TEST);
		glClear(GL_DEPTH_BUFFER_BIT);
		glDepthFunc(GL_LESS);

		dirShadowShader.Activate();
		dirShadowShader.setMat4("lightVP", dirLightVP);
		dirShadowShader.setMat4("model", clothModel);
		cloth.DrawShadow(dirShadowShader, camera);

		// PRIMARY
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, width, height);

		// Specify the color of the background
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

		// Clean the back buffer and assign the new color to it
		glClear(GL_COLOR_BUFFER_BIT);
		glClear(GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);

		
		camera.updateMatrix(45.0f, 0.1f, 100.0f);
		camera.Input(window, dt);

		shader.Activate();
		shader.setVec3("camDir", camera.GetDirection());
		shader.setMat4("model", floorModel);
		shader.setBool("flashLight", camera.GetFlashlight());
		shader.setMat4("dirLightVP", dirLightVP);
		shader.setInt("shadowMap", 2);
		glActiveTexture(GL_TEXTURE2);
		shadowFBO.BindTex2D();
		floor.Draw(shader, camera);

		// CLOTH
		cloth.SetColor(glm::vec3(1.0f, 0.0f, 0.0f));
		if(glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
			cloth.Simulate();

		GLclearError();
		cloth.Draw(camera, fill);
		GLcheckError();
		glfwSwapBuffers(window);

		// Take care of all GLFW events
		glfwPollEvents();
	}
	shader.Delete();
	dirShadowShader.Delete();

	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}



