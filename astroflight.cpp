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
#include "level.hpp"

// Debug console output
#include <iostream>		

// Level list
#include <vector>

#if __has_include(<filesystem>)
#include <filesystem>
namespace fs = std::filesystem;
#elif __has_include(<experimental/filesystem>)
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif



// Settings
int xpos, ypos;						// Last window position before switching to fullscreen mode
double cursorX, cursorY;			// Last cursor position upon left click
const GLuint SCR_WIDTH = 1280;		// Default window width
const GLuint SCR_HEIGHT = 720;		// Default window height
const glm::mat4 projection = glm::ortho(0.0f, (float)SCR_WIDTH, 0.0f, (float)SCR_HEIGHT);

// Flags
bool windowed = true;
bool wireframe = false;
bool clicked = false;

// Selected object for gravity field
unsigned int planetID = -1;
unsigned int moonID = -1;

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
			//glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_FALSE);
			windowed = false;
			const GLFWvidmode * mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
			glfwSetWindowMonitor(window, NULL, 0, 0, mode->width, mode->height, mode->refreshRate);
		}
		else
		{
			//glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_TRUE);
			glfwSetWindowMonitor(window, NULL, xpos, ypos, SCR_WIDTH, SCR_HEIGHT, 0);
			glfwSetWindowPos(window, xpos, ypos);
			windowed = true;
		}
	}
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	// Toggle gravity field on left mouse button
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		int width, height;

		glfwGetCursorPos(window, &cursorX, &cursorY);
		glfwGetWindowSize(window, &width, &height);

		cursorX *= SCR_WIDTH / (float)width;
		cursorY *= SCR_HEIGHT / (float)height;

		clicked = true;
	}
}


// Set up a new shader with the defined projection matrix
// ------------------------------------------------------
Shader addShader(const GLchar * vertexFileName, const GLchar * fragmentFileName)
{
	Shader newShader(vertexFileName, fragmentFileName);
	newShader.use();
	newShader.setMat4("projection", projection);
	return newShader;
}


// Get a list of valid levels contained in "levels" folder
// -------------------------------------------------------
std::vector<Level*> loadLevels(const std::string fileName = "_loadAll")
{
	std::vector<Level*> levels;

	if (fileName == "_loadAll")
	{
		// Load all levels
		for (const auto & file : fs::directory_iterator("levels"))
		{
			std::string filePath = file.path().string();
			if (filePath.substr(filePath.length() - 4) == ".lvl")
			{
				Level * temp = new Level(filePath);
				if (temp->isValid())
					levels.push_back(temp);
				else
					delete temp;
			}
				
		}
	}
	else
	{
		// Load level defined in command line parameter
		Level * temp = new Level("levels/" + fileName + ".lvl");
		if (temp->isValid())
			levels.push_back(temp);
		else
			delete temp;
	}

	return levels;
}


// Toggle gravity fields of planets/moons upon left mouse click
void toggleFields(Level level)
{
	glm::vec2 cursorPos = glm::vec2(cursorX, SCR_HEIGHT - cursorY);

	std::vector<Planet> planets(level.getPlanets());
	for (unsigned int i = 0; i < planets.size(); ++i)
	{
		if (glm::distance(planets[i].getPosition(), cursorPos) <= planets[i].getRadius())
		{
			if (planetID != i)
			{
				planetID = i;
				moonID = -1;
			}
			else
				planetID = -1;
			clicked = false;
			return;
		}
	}

	std::vector<Moon> moons(level.getMoons());
	for (unsigned int i = 0; i < moons.size(); ++i)
	{
		if (glm::distance(moons[i].getPosition(), cursorPos) <= moons[i].getRadius())
		{
			if (moonID != i)
			{
				moonID = i;
				planetID = -1;
			}
			else
				moonID = -1;
			clicked = false;
			return;
		}
	}

	clicked = false;
}


int main(int argc, char * argv[])
{
	// Load level list
	// ---------------
	std::vector<Level*> levels;

	if (argc == 1)
		levels = loadLevels();
	else
		levels = loadLevels(argv[1]);

	std::cout << "Loaded levels: " << levels.size() << std::endl;
	for (Level *level : levels)
	{
		std::cout << level->getName() << std::endl;
		level->genPhysics();
	}

	if (levels.size() == 0)
	{
		std::cout << "Directory 'level' must contain at least 1 valid level with file extension '.lvl'\n";
		std::cout << "Press Enter to exit";
		std::cin >> argv[0];
		return 0;
	}

	Level level = *levels[0];


	// GLFW: Setup
	// -----------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	glfwWindowHint(GLFW_SAMPLES, 4);

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
	glfwSetMouseButtonCallback(window, mouseButtonCallback);


	// GLAD: loading OpenGL function pointers
	// --------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// Enabling z-buffer and blending
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glEnable(GL_MULTISAMPLE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Building necessary shader programs
	// ----------------------------------
	Shader shaderField = addShader("vGradient", "fGravField");		// Gravitational fields
	Shader shaderPlanet = addShader("vSimple", "fPlanet");			// Planets and trabants


	// Render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		// Toggle gravity field
		if (clicked)
			toggleFields(level);

		// Draw objects
		for (auto pm : level.getPointMasses())
			pm.drawField(shaderField);
		for (auto planet : level.getPlanets())
			planet.draw(shaderPlanet);
		for (auto moon : level.getMoons())
			moon.draw(shaderPlanet);

		// Draw gravity field
		if (planetID != -1)
			level.getPlanets()[planetID].drawField(shaderField);
		if (moonID != -1)
			level.getMoons()[moonID].drawField(shaderField);


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