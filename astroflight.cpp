#include <glad/glad.h>	// OpenGL headers
#include <GLFW/glfw3.h>	// Window & input handling
#include "shader.hpp"	// Class for compiling & linking shaders

// Transformations
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

// Game data
#include "shapes.hpp"
#include "game_objects.hpp"

// Debug console output
#include <iostream>		


// Settings
int xpos, ypos;						// Last window position before switching to fullscreen mode
const GLfloat SCR_WIDTH = 1280.0f;	// Default window width
const GLfloat SCR_HEIGHT = 720.0f;	// Default window height
const glm::mat4 projection = glm::ortho(0.0f, SCR_WIDTH, 0.0f, SCR_HEIGHT);

// Flags
bool windowed = true;
bool wireframe = false;


// GLFW: Callback function for window size
// -----------------------------------------------------------------------
void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}


// GLFW: Callback function for window position
// -------------------------------------------
void windowPosCallback(GLFWwindow* window, int x, int y)
{
	if (windowed)
	{
		xpos = x;
		ypos = y;
	}
}


// GLFW: Callback function for inputs
// ----------------------------------
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// Exit program on ESC
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	// Toggle wireframe mode with W
	if (key == GLFW_KEY_W && action == GLFW_PRESS)
	{
		if (wireframe)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			wireframe = false;
		}
		else
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			wireframe = true;
		}
	}

	// Toggle fullscreen mode with F
	if (key == GLFW_KEY_F && action == GLFW_PRESS)
	{
		if (windowed)
		{
			windowed = false;
			const GLFWvidmode * mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
			glfwSetWindowMonitor(window, NULL, 0, 0, mode->width, mode->height, mode->refreshRate);
		}
		else
		{
			glfwSetWindowMonitor(window, NULL, xpos, ypos, SCR_WIDTH, SCR_HEIGHT, 0);
			glfwSetWindowPos(window, xpos, ypos);
			windowed = true;
		}
	}
}


int main()
{
	// GLFW: Setup
	// -----------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	//  Window creation
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "AstroFlight", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwGetWindowPos(window, &xpos, &ypos);

	// Setting callback functions
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
	glfwSetWindowPosCallback(window, windowPosCallback);
	glfwSetKeyCallback(window, keyCallback);


	// GLAD: loading OpenGL function pointers
	// --------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// Enabling z-buffer and blending
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Building necessary shader programs
	// ----------------------------------
	Shader shaderGradient("vGradient", "fGradient");
	

	// set up vertex data and buffers and configure vertex attributes
	// --------------------------------------------------------------
	GLfloat * vertices = getDisk();

	GLuint VBO, VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * nVertices * 2, vertices, GL_STATIC_DRAW);
	// position attribute
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// get game objects
	// ----------------
	PointMass test(100.0f, 400.0f, 300.0f);

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		// render
		// ------
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// draw
		shaderGradient.use();
		shaderGradient.setMat4("projection", projection);

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(test.getPosition().x, test.getPosition().y, 0));
		model = glm::scale(model, glm::vec3(test.getGravRadius(), test.getGravRadius(), 0.0f));
		shaderGradient.setMat4("model", model);

		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLE_FAN, 0, nVertices);

		// glfw: swap buffers and poll IO events
		// -------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	
	// glfw: terminate, clearing all previously allocated GLFW resources
	// -----------------------------------------------------------------
	glfwTerminate();
	return 0;
}