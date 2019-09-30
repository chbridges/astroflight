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
#include "gui.hpp"

// Debug console output
#include <iostream>	

// Star randomizer
#include <cstdlib>

// Level list
#include <vector>
#include <algorithm>

#if __has_include(<filesystem>)
#include <filesystem>
namespace fs = std::experimental::filesystem;
#elif __has_include(<experimental/filesystem>)
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif

constexpr auto NEXT_LEVEL = 999;

// Settings
const GLuint SCR_WIDTH = 1280;		// Default window width
const GLuint SCR_HEIGHT = 720;		// Default window height
const glm::mat4 projection = glm::ortho(0.0f, (float)SCR_WIDTH, 0.0f, (float)SCR_HEIGHT);

// OpenGL seems to behave differently on UNIX / Windows
#ifdef _WIN32
float physicsFPS = 60.0f;		// Updates per second. 60 ensures fluent visuals, higher values increase game speed
#else
float physicsFPS = 120.0f;
#endif

float speedMultiplicator = 1.0f;
const unsigned int maxBoxes = 3;

// Mouse and window positions
int windowX, windowY;				// Last window position before switching to fullscreen mode
double cursorX, cursorY;			// Last cursor position upon left click

// Flags
bool windowed = true;
bool wireframe = false;
bool clicked = false;
bool drawTrajectory = false;
bool nextLevel = false;
bool restartLevel = false;
bool showFPS = false;
bool showCOM = false;				// Center of Mass
bool turnLeft = false;
bool turnRight = false;
bool increaseSpeed = false;
bool decreaseSpeed = false;
bool boost = false;
bool launch = false;
bool gameOver = false;
bool gameWon = false;
bool signalLost = false;
bool pause = true;
bool gui = true;
int precisionMode = 0;

// Selected object for gravity field
int planetID = -1;
int moonID = -1;
int blackHoleID = -1;

// Level management
std::vector<std::string> levelList;
unsigned int levelID = 0;

// Tick rate management
float physicsTickRate = 1.0f / physicsFPS;			// Physics updates per second
unsigned int frameCount = 0;						// Frames per second
double currentTime = glfwGetTime();					// For measuring time intervals
double lastSecond = currentTime;					// Update every second to measure FPS
double lastTick = currentTime;						// Update every tick
double outOfBounds = 0.0f;							// Measures how long player has been outside the window
int counter = 0;

// GUI
unsigned int speedCountdown = 0;
int currentFPS = 0;


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
			glfwSetWindowMonitor(window, glfwGetPrimaryMonitor(), 0, 0, mode->width, mode->height, mode->refreshRate);
		}
		else
		{
			//glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_TRUE);
			glfwSetWindowMonitor(window, NULL, windowX, windowY, SCR_WIDTH, SCR_HEIGHT, 0);
			glfwSetWindowPos(window, windowX, windowY);
			windowed = true;
		}
	}

	// Toggle GUI with G
	if (key == GLFW_KEY_G && action == GLFW_PRESS)
		gui = !gui;

	// Toggle trajectory with T
	if (key == GLFW_KEY_T && action == GLFW_PRESS)
		drawTrajectory = !drawTrajectory;

	// Toggle center of mass with C
	if (key == GLFW_KEY_C && action == GLFW_PRESS)
		showCOM = !showCOM;

	// Skip to next level with N
	if (key == GLFW_KEY_N && action == GLFW_PRESS)
		nextLevel = true;

	// Restart current level with R
	if (key == GLFW_KEY_R && action == GLFW_PRESS)
	{
		nextLevel = true;
		restartLevel = true;
	}

	// Toggle FPS counter with D
	if (key == GLFW_KEY_D && action == GLFW_PRESS)
		showFPS = !showFPS;

	// Pause game with P
	if (key == GLFW_KEY_P && action == GLFW_PRESS)
		pause = !pause;

	// Change game speed
	if (key == GLFW_KEY_RIGHT_BRACKET && action == GLFW_PRESS && speedMultiplicator < 4.0f)
	{
		speedMultiplicator *= 2.0f;
		physicsFPS *= 2.0f;
		physicsTickRate = 1.0f / physicsFPS;
		speedCountdown = 2;
	}
	if (key == GLFW_KEY_SLASH && action == GLFW_PRESS && speedMultiplicator > 0.25f)
	{
		speedMultiplicator *= 0.5f;
		physicsFPS *= 0.5f;
		physicsTickRate = 1.0f / physicsFPS;
		speedCountdown = 2;
	}

	// Rotating the space ship with arrow keys
	if (key == GLFW_KEY_LEFT && action == GLFW_PRESS)
		turnLeft = true;
	else if (key == GLFW_KEY_LEFT && action == GLFW_RELEASE)
		turnLeft = false;
	if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
		turnRight = true;
	else if (key == GLFW_KEY_RIGHT && action == GLFW_RELEASE)
		turnRight = false;
	if ((key == GLFW_KEY_LEFT_SHIFT || key == GLFW_KEY_RIGHT_SHIFT) && action == GLFW_PRESS)
		++precisionMode;
	else if ((key == GLFW_KEY_LEFT_SHIFT || key == GLFW_KEY_RIGHT_SHIFT) && action == GLFW_RELEASE)
		precisionMode = precisionMode > 0 ? precisionMode-1 : 0;

	// Increment/Decrement launch speed or use boost
	if (key == GLFW_KEY_UP && action == GLFW_PRESS)
		increaseSpeed = true;
	if (key == GLFW_KEY_DOWN && action == GLFW_PRESS)
		decreaseSpeed = true;

	// Launch or drop box
	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
		launch = true;
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
				blackHoleID = -1;
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
				blackHoleID = -1;
			}
			else
				moonID = -1;
			return;
		}
	}

	std::vector<BlackHole> blackHoles(level.getBlackHoles());
	for (unsigned int i = 0; i < blackHoles.size(); ++i)
	{
		if (glm::distance(blackHoles[i].getPosition(), cursorPos) <= blackHoles[i].getRadius())
		{
			if (blackHoleID != i)
			{
				blackHoleID = i;
				planetID = -1;
				moonID = -1;
			}
			else
				blackHoleID = -1;
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

	std::sort(levelList.begin(), levelList.end());

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
	std::cout << "Loading level: " << level.getName() << std::endl;
}

// Generate randomized star background
// -----------------------------------
std::vector<Star> generateStars(const int multiplier = 2)
{
	std::vector<Star> stars;
	std::srand((unsigned int)glfwGetTime());
	GLfloat offset1, offset2;

	for (int w = 0; w < SCR_WIDTH; w += SCR_WIDTH / (16 * multiplier))
	{
		for (int h = 0; h < SCR_HEIGHT; h += SCR_HEIGHT / (9 * multiplier))
		{
			offset1 = (float)(std::rand() % 2000) / 1000.0f - 1.0f;
			offset2 = (float)(std::rand() % 2000) / 1000.0f - 1.0f;

			stars.push_back(Star(glm::vec2(w,h), glm::vec2(offset1, offset2)));

			if (w == 0 || w == SCR_WIDTH || h == 0 || h == SCR_HEIGHT)
			{
				glm::vec2 lastPos = stars.back().getPosition();
				if (lastPos.x <= 0 || lastPos.x >= SCR_WIDTH || lastPos.y <= 0 || lastPos.y >= SCR_HEIGHT)
					stars.pop_back();
			}
		}
	}

	return stars;
}

void drawStars(const std::vector<Star>& stars, const Shader& shader)
{
	// Getting the vertices of the disk
	GLfloat * vertices = getLowPolyDisk();

	// Generating and binding buffers
	GLuint VBO, VAO;
	glGenBuffers(1, &VBO);

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * nVerticesLow * 2, vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(VAO);

	// Loading the shader and transforming the disk
	shader.use();

	glm::vec2 position, offset;
	glm::mat4 model;
	GLfloat radius, brightness, brightnessVar, colorMod;

	for (auto & star : stars)
	{
		position = star.getPosition();
		offset = star.getOffset();
		radius = star.getRadius();
		brightness = star.getBrightness();
		colorMod = radius+brightness < 1.8f ? 0.0f : 0.4f;
		brightnessVar = brightness + 0.1f * sin((GLfloat)glfwGetTime() + offset.x * 10.0f);

		if (offset.y < -0.9f)		// very few yellow stars
			shader.setVec3("color", glm::vec3(1.0f, 1.0f, 1.0f - colorMod) * brightnessVar);
		else if (offset.y < -0.7f)	// very few purple stars
			shader.setVec3("color", glm::vec3(1.0f, 1.0f - colorMod, 1.0f) * brightnessVar);
		else if (offset.y < 0.7f)	// little more red stars
			shader.setVec3("color", glm::vec3(1.0f, 1.0f - colorMod, 1.0f - colorMod) * brightnessVar);
		else						// many blue stars
			shader.setVec3("color", glm::vec3(1.0f - colorMod, 1.0f - colorMod, 1.0f) * brightnessVar);

		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(position.x, position.y, -0.9f));
		model = glm::scale(model, glm::vec3(radius + 0.5f * (brightnessVar + colorMod), radius + 0.5f * (brightnessVar + colorMod), 0.0f));
		shader.setMat4("model", model);
		
		glDrawArrays(GL_TRIANGLE_FAN, 0, nVerticesLow);
	}

	// Clearing the buffers to avoid memory leak
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
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
	std::cout << "Loading level: " << level.getName() << std::endl;
	level.genPhysics();
	SpaceShip player(level.getPlanets()[0]);
	Trajectory trajectory(player, level.getPhysics(), 2000);
	CenterOfMass centerOfMass;
	Flag flag(level.getPlanets()[1]);
	std::vector<Star> stars = generateStars();
	glm::vec2 playerPosition = player.getPosition();

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
	glLineWidth(2);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_MULTISAMPLE);


	// Building necessary shader programs
	// ----------------------------------
	Shader shaderSimple = addShader("vDefault", "fSimple");			// Black holes, space ship, trajectory
	Shader shaderLighting = addShader("vDefault", "fLighting");		// Planets, moons
	Shader shaderField = addShader("vDefault", "fGravField");		// Gravitational fields
	Shader shaderAtmosphere = addShader("vDefault", "fAtmosphere");	// Atmosphere of planets and moons
	Shader shaderGradient = addShader("vDefault", "fGradient");		// Stars, center of mass and event horizon of black holes
	Shader shaderText = addShader("vText", "fText");				// GUI text
	Shader shaderBox = addShader("vGUI", "fAlpha");					// GUI text box

	// Lighting setup
	shaderLighting.use();
	shaderLighting.setVec3("light.color", glm::vec3(255, 255, 255));
	shaderLighting.setVec3("light.direction", glm::vec3(1.0f, -1.0f, 0.0f));

	// Loading GUI
	GUI::textInit();
	std::string guiGameSpeed, guiLaunchSpeed, guiLaunchAngle, guiLevelName, guiScore, guiGameOver, guiMass, guiFPS;
	GLuint infoBoxAddonsX = level.getName().length();
	GLuint infoBoxAddonsY = 1;
	GLuint counterOffset = 0;
	glm::vec3 guiTextColor = glm::vec3(0.5f, 0.8f, 0.2f);
	glm::vec4 guiBoxColor = glm::vec4(0.0f, 0.0f, 0.0f, 0.2f);

	// Game loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{	
		currentTime = glfwGetTime();

		// Executes once per second
		if (currentTime - lastSecond >= 1.0f)
		{
			lastSecond = currentTime;
			currentFPS = frameCount;
			frameCount = 0;
			if (speedCountdown)
				--speedCountdown;
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
			{
				changeLevel(level, levelID);
			}
			else
			{
				changeLevel(level);
				drawTrajectory = false;
				stars = generateStars();
			}
			level.genPhysics();
			player.setPlanet(level.getPlanets()[0], true);
			flag.setPlanet(level.getPlanets()[1]);
			trajectory.setPointMassPointer(level.getPhysics());
			trajectory.update();
			pause = true;
			nextLevel = false;
			restartLevel = false;
			gameOver = false;
			gameWon = false;
			signalLost = false;
			planetID = -1;
			moonID = -1;
			blackHoleID = -1;
		}

		if (launch)
		{
			pause = false;

			if (player.getLaunchState() == 0)
				player.launchProgress();
			else if (player.getLaunchState() < 4)
			{
				if (level.getBoxes().size() < maxBoxes)
					level.getBoxes().push_back(Box(player, level.getPhysics()));
			}
				
			launch = false;
		}

		if (increaseSpeed)
		{
			if (player.getLaunchState() == 0)
				player.adjustSpeed(true, precisionMode);
			else if (player.getLaunchState() == 2)
				player.launchProgress();
			increaseSpeed = false;
		}

		if (decreaseSpeed)
		{
			if (player.getLaunchState() == 0)
				player.adjustSpeed(false, precisionMode);
			decreaseSpeed = false;
		}
		
		// Check if player is out of bounds
		playerPosition = player.getPosition();
		if (playerPosition.x < 0-spaceShipSize || playerPosition.x > SCR_WIDTH+spaceShipSize || playerPosition.y < 0-spaceShipSize || playerPosition.y > SCR_HEIGHT+spaceShipSize)
		{
			if (outOfBounds == 0.0f)
			{
				outOfBounds = glfwGetTime();
			}
			else
			{
				if ((currentTime - outOfBounds) * speedMultiplicator >= 5.0f)
				{
					gameOver = true;
					signalLost = true;
				}
			}
		}
		else
		{
			outOfBounds = 0.0f;
		}

		// Move objects
		if (currentTime - lastTick > physicsTickRate)
		{
			lastTick = currentTime;

			if (turnLeft)
				player.rotate(false, precisionMode);
			if (turnRight)
				player.rotate(true, precisionMode);

			if (!pause)
				level.updatePhysics();
			if (!gameOver && !pause || player.getLaunchState() == 0)
				player.move(level.getPhysics());
			if (player.getLaunchState() == 0)
				trajectory.update();
			if (showCOM)
				centerOfMass.update(level.getPhysics());
			flag.move();

			if (!gameOver && player.getLaunchState() == 4)
			{
				gameOver = true;

				if (glm::distance(player.getPosition(), level.getPlanets()[1].getPosition()) <= level.getPlanets()[1].getRadius() + collisionShip)
				{
					// won
					level.updateScore(200 - player.hasBoosted() * 100);
					gameWon = true;
					// save score
				}
				else
				{
					// lost
					gameWon = false;
				}
			}

			// Check if one of the boxes hit a planet
			for (auto & box : level.getBoxes())
			{
				if (box.hasLanded() && !box.isProcessed())
				{
					for (auto & planet : level.getPlanets())
					{
						if (planet.getTerraforming() == 1 && glm::distance(box.getPosition(), planet.getPosition()) <= planet.getRadius() + collisionBox)
						{
							level.updateScore(100);
							break;
						}
					}
					for (auto & moon : level.getMoons())
					{
						if (moon.getTerraforming() == 1 && glm::distance(box.getPosition(), moon.getPosition()) <= moon.getRadius() + collisionBox)
						{
							level.updateScore(200);
							break;
						}
					}
					box.process();
				}
			}
		}

		// Clear buffers
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Draw stars (z = -0.9f)
		drawStars(stars, shaderGradient);

		// Draw gravity field (z = -0.5f)
		if (planetID != -1)
			level.getPlanets()[planetID].drawField(shaderField);
		if (moonID != -1)
			level.getMoons()[moonID].drawField(shaderField);
		if (blackHoleID != -1)
			level.getBlackHoles()[blackHoleID].drawField(shaderField);

		// Draw trajectory (z = -0.25f)
		if (drawTrajectory)
			trajectory.draw(shaderSimple);

		// Draw objects (z = 0.0f)
		player.draw(shaderSimple);
		flag.draw(shaderSimple);

		for (auto pm : level.getPointMasses())
			pm.drawField(shaderField);
		for (auto planet : level.getPlanets())
			planet.draw(shaderLighting);
		for (auto & moon : level.getMoons())
			moon.draw(shaderLighting);
		for (auto & bh : level.getBlackHoles())
			bh.draw(shaderSimple, shaderGradient);
		for (auto & box : level.getBoxes())
			box.draw(shaderSimple);

		// Draw atmospheres (z = 0.5f)
		for (auto planet : level.getPlanets())
			planet.drawAtmosphere(shaderAtmosphere);
		for (auto & moon : level.getMoons())
			moon.drawAtmosphere(shaderAtmosphere);

		// Draw center of mass
		if (showCOM)
			centerOfMass.draw(shaderGradient);


		// Draw GUI
		// --------
		if (gui)
		{
			// Launch settings box
			GUI::renderBox(shaderBox, 5, 3, 258, 60, guiBoxColor);

			// Info box
			infoBoxAddonsX = level.getName().length();
			infoBoxAddonsY = showFPS + (pause || speedCountdown > 0);
			GUI::renderBox(shaderBox, 5, SCR_HEIGHT-67-infoBoxAddonsY*30, 142+infoBoxAddonsX*9, 60+infoBoxAddonsY*30, guiBoxColor);

			// Launch angle
			guiLaunchAngle = std::to_string(glm::degrees(player.getLaunchAngle())+0.01f);
			guiLaunchAngle = guiLaunchAngle.substr(0, guiLaunchAngle.length()-5);
			if (guiLaunchAngle == "360.0")
				guiLaunchAngle = "0.0";
			guiLaunchAngle = std::string("Launch angle: ").append(guiLaunchAngle);
			GUI::renderText(shaderText, guiLaunchAngle, 10, 10, 0.5f, guiTextColor);

			// Launch speed
			guiLaunchSpeed = std::to_string(2*player.getLaunchSpeed()-1);
			guiLaunchSpeed = std::string("Launch speed:  ").append(guiLaunchSpeed.substr(0, guiLaunchSpeed.length()-5));
			GUI::renderText(shaderText, guiLaunchSpeed, 10, 40, 0.5f, guiTextColor);

			// Level name
			guiLevelName = std::string("Level: ").append(level.getName());
			GUI::renderText(shaderText, guiLevelName, 10, SCR_HEIGHT-30, 0.5f, guiTextColor);

			// Score
			guiScore = std::string("Score: ").append(std::to_string(level.getScore()));
			GUI::renderText(shaderText, guiScore, 10, SCR_HEIGHT-60, 0.5f, guiTextColor);

			// Framerate
			if (showFPS)
			{
				guiFPS = std::string("FPS: ").append(std::to_string(currentFPS));
				GUI::renderText(shaderText, guiFPS, 10, SCR_HEIGHT-90, 0.5f, guiTextColor);
			}

			// Game speed multiplier and pause notification
			if (speedCountdown)
				guiGameSpeed = std::string("Speed: ").append(std::to_string(speedMultiplicator).substr(0,4).append("x"));
			else if (pause)
				guiGameSpeed = std::string("Paused");
			else
				guiGameSpeed = std::string("");
			GUI::renderText(shaderText, guiGameSpeed, 10, SCR_HEIGHT-90-(showFPS)*30, 0.5f, guiTextColor);


			// Game won/lost message and out-of-bounds-counter
			if (gameOver)
			{
				if (gameWon)
				{
					//GUI::renderBox(shaderBox, 520, 335, 242, 50, guiBoxColor);
					GUI::renderText(shaderText, "You won", 529, 345, 1.0f, guiTextColor);
				}
				else if (signalLost)
				{
					//GUI::renderBox(shaderBox, 490, 335, 304, 50, guiBoxColor);
					GUI::renderText(shaderText, "Signal lost", 496, 345, 1.0f, guiTextColor);
				}
				else
				{
					//GUI::renderBox(shaderBox, 524, 335, 234, 50, guiBoxColor);
					GUI::renderText(shaderText, "You lost", 533, 345, 1.0f, guiTextColor);
				}
			}
			else if (outOfBounds != 0.0f)
			{
				counter = (int)(6.0f - (currentTime - outOfBounds) * speedMultiplicator);
				//GUI::renderBox(shaderBox, 618, 335, 49, 50, guiBoxColor);

				switch (counter)
				{
				case 1:
					counterOffset = 10;
					break;
				case 2:
					counterOffset = 0;
					break;
				case 3:
					counterOffset = 2;
					break;
				case 4:
					counterOffset = 2;
					break;
				case 5:
					counterOffset = 3;
					break;
				default:
					std::cout << "Error: Invalid outOfBounds counter" << std::endl;
				}

				GUI::renderText(shaderText, std::to_string(counter), 626 + counterOffset, 345, 1.0f, guiTextColor);
				
			}
		}

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