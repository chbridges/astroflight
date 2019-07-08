#include <glad/glad.h>	// OpenGL headers
#include <GLFW/glfw3.h>	// Window & input handling
#include "shader.hpp"	// Class for compiling & linking shaders

// Transformations
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

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

constexpr auto NEXT_LEVEL = 999;

// Settings
const GLuint SCR_WIDTH = 1280;		// Default window width
const GLuint SCR_HEIGHT = 720;		// Default window height
const glm::mat4 projection = glm::ortho(0.0f, (float)SCR_WIDTH, 0.0f, (float)SCR_HEIGHT);

const float physicsFPS = 60.0f;		// Game speed, independent of the rendering speed

// Mouse and window positions
int windowX, windowY;				// Last window position before switching to fullscreen mode
double cursorX, cursorY;			// Last cursor position upon left click

// Flags
bool windowed = true;
bool wireframe = false;
bool clicked = false;
bool nextLevel = false;
bool restartLevel = false;
bool showFPS = false;

// Selected object for gravity field
unsigned int planetID = -1;
unsigned int moonID = -1;

// Level management
std::vector<std::string> levelList;
unsigned int levelID = 0;

// Tick rate management
const float physicsTickRate = 1.0f / physicsFPS;	// Physics updates per second
unsigned int frameCount = 0;						// Frames per second
double currentTime = glfwGetTime();					// For measuring time intervals
double lastSecond = currentTime;					// Update every second to measure FPS
double lastTick = currentTime;						// Update every tick


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
		windowX = x;
		windowY = y;
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
			glfwSetWindowMonitor(window, NULL, windowX, windowY, SCR_WIDTH, SCR_HEIGHT, 0);
			glfwSetWindowPos(window, windowX, windowY);
			windowed = true;
		}
	}

	// Skip to next level with N
	if (key == GLFW_KEY_N && action == GLFW_PRESS)
	{
		nextLevel = true;
	}

	// Restart current level with R
	if (key == GLFW_KEY_R && action == GLFW_PRESS)
	{
		nextLevel = true;
		restartLevel = true;
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

		cursorX *= (float)SCR_WIDTH / (float)width;
		cursorY *= (float)SCR_HEIGHT / (float)height;

		clicked = true;
	}
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
			return;
		}
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
std::vector<std::string> loadLevelList(const std::string fileName = "_loadAll")
{
	std::vector<std::string> levelList;

	if (fileName == "_loadAll")
	{
		// Load all levels and check validity
		for (const auto & file : fs::directory_iterator("levels"))
		{
			std::string filePath = file.path().string();
			if (filePath.substr(filePath.length() - 4) == ".lvl")
			{
				Level temp(filePath);
				if (temp.isValid())
					levelList.push_back(temp.getName());
			}
				
		}
	}
	else
	{
		// Load level defined in command line parameter and check validity
		Level temp("levels/" + fileName + ".lvl");
		if (temp.isValid())
			levelList.push_back(temp.getName());
	}

	return levelList;
}


// Load a level by name
// --------------------
Level loadLevelByName(const std::string name)
{
	return Level("levels/" + name + ".lvl");
}


// Load a level by ID or next level
// --------------------------------
void changeLevel(Level& level, unsigned int ID = NEXT_LEVEL, bool shuffle = false)
{
	if (shuffle)
	{
		// open random level
	}
	else if (ID == NEXT_LEVEL)
	{
		if (++levelID == levelList.size())
			levelID = 0;
	}
	else
	{
		if (ID < levelList.size())
			levelID = ID;
	}
	
	level = loadLevelByName(levelList[levelID]);
	std::cout << "Loaded level: " << level.getName() << std::endl;
}


int main(int argc, char * argv[])
{
	// Load level list
	// ---------------
	if (argc == 1)
		levelList = loadLevelList();
	else
		levelList = loadLevelList(argv[1]);

	std::cout << "Loaded levels: " << levelList.size() << std::endl;
	for (auto name  : levelList)
	{
		std::cout << name << std::endl;
	}

	if (levelList.size() == 0)
	{
		std::cout << "Directory 'level' must contain at least 1 valid level with file extension '.lvl'\n";
		std::cout << "Press Enter to exit";
		std::cin >> argv[0];
		return 0;
	}

	Level level = loadLevelByName(levelList[levelID]);
	std::cout << "Loaded level: " << level.getName() << std::endl;

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
	glfwGetWindowPos(window, &windowX, &windowY);

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
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_MULTISAMPLE);


	// Building necessary shader programs
	// ----------------------------------
	Shader shaderField = addShader("vGradient", "fGravField");		// Gravitational fields
	Shader shaderPlanet = addShader("vSimple", "fPlanet");			// Planets and moons


	// Game loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{	
		currentTime = glfwGetTime();

		if (currentTime - lastSecond >= 1.0f)
		{
			lastSecond = currentTime;
			std::cout << "FPS: " << frameCount << std::endl;
			frameCount = 0;
		}
		else
			++frameCount;


		// Handle inputs
		if (clicked)
		{
			toggleFields(level);
			clicked = false;
		}
			
		if (nextLevel)
		{
			if (restartLevel)
				changeLevel(level, levelID);
			else
				changeLevel(level);
			planetID = -1;
			moonID = -1;
			nextLevel = false;
			restartLevel = false;
		}
			

		// Move objects
		if (currentTime - lastTick > physicsTickRate)
		{
			lastTick = currentTime;

			for (auto & pm : level.getPointMasses())
				pm.move();
			for (auto & planet : level.getPlanets())
				planet.move();
			for (auto & moon : level.getMoons())
				moon.move();
		}

		// Clear buffers
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Draw objects
		for (auto pm : level.getPointMasses())
			pm.drawField(shaderField);
		for (auto planet : level.getPlanets())
			planet.draw(shaderPlanet);
		for (auto & moon : level.getMoons())
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