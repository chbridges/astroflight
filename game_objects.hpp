#ifndef GAME_OBJECTS_H
#define GAME_OBJECTS_H

#include <glad/glad.h>	// OpenGL headers
#include "glm/glm.hpp"	// Vectors and transformation matrices
#include "shader.hpp"
#include "shapes.hpp"

#include <vector>
#include <cmath>
#include <algorithm>

// Constants
const GLfloat gameSpeed = 0.5f;
const GLfloat gravityScale = 1.0f;				// Scales gravity force and keeps draw distance consistent
const GLfloat G = gameSpeed * 6.6743f * gravityScale;		// Scaled gravitational constant
const GLfloat epsilon = 0.01f * gravityScale;	// Minimal gravitational force to visualize
const GLfloat schwartzschild = 0.067f*G;			// Multiply with mass for Schwartzschild radius, c^-2 dropped for gameplay reasons
const GLfloat spaceShipSize = 20.0f;
const GLfloat rotationSpeed = 1.0f / 180.0f * pi;
const GLfloat boostPower = 2.0f;
const GLfloat boxSize = 5.0f;
const GLfloat boxRotation = 1.0f / 180.0f * pi;
const GLfloat flagSize = 15.0f;
const GLfloat collisionScale = 0.5f;			// Lower value = smaller collision box, negative values possible
const GLfloat collisionShip = collisionScale * spaceShipSize * 0.17f;
const GLfloat collisionBox = collisionScale * boxSize * 0.67f;
const GLfloat atmosphereScale = 1.1f;



// The core of all physics objects
// -------------------------------
class PointMass
{
protected:
	GLfloat mass;
	GLfloat radius;
	GLfloat gravRadius;
	glm::vec2 position;
	glm::vec2 velocity;
	glm::vec2 acceleration;

	// Calculating the gravitational force applied by another point mass, doesn't consider own mass
	// --------------------------------------------------------------------------------------------
	glm::vec2 gravitationalAcceleration(const PointMass& other) const
	{
		glm::vec2 rv = other.getPosition() - this->position;	// Distance vector pointing to the other mass
		GLfloat rl = glm::length(rv);							// Length of distance vector
		return G * other.getMass() / (rl * rl * rl) * rv;
	}

	// Calculating the centrifugal force applied by another point mass
	// ---------------------------------------------------------------
	glm::vec2 centrifugalAcceleration(const PointMass& other) const
	{
		glm::vec2 rv = this->position - other.getPosition();	// Distance vector pointing away from the other mass
		GLfloat angle = glm::dot(rv, this->velocity);			// Apply centrifugal force if velocity vector is orthogonal
		GLfloat vl = glm::length(velocity);
		return angle < epsilon ? vl * vl / glm::length(rv) * glm::normalize(rv) : glm::vec2(0.0f, 0.0f);
	}


	// Acceleration depends on which forces are supposed to affect the object
	virtual void accelerate() {}

	// Drawing a disk for a planet (z = 0) or gravity field (z = 0.5)
	// --------------------------------------------------------------
	void drawDisk(const Shader& shader, const GLfloat radius, const GLfloat z = 0.0f) const
	{
		// Getting the vertices of the disk
		GLfloat * vertices = getDisk();

		// Generating and binding buffers
		GLuint VBO, VAO;
		glGenBuffers(1, &VBO);

		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * nVertices * 2, vertices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		// Loading the shader and transforming the disk
		shader.use();

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(position.x, position.y, z));
		model = glm::scale(model, glm::vec3(radius, radius, 0.0f));
		shader.setMat4("model", model);

		// Drawing the disk
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLE_FAN, 0, nVertices);

		// Clearing the buffers to avoid memory leak
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
	}


public:
	// Constructor
	// Arguments: mass, position.x, position.y
	// Optional: velocity.x, velocity.y
	// ---------------------------------------
	PointMass(const GLfloat mass, const GLfloat px, const GLfloat py, const GLfloat vx = 0, const GLfloat vy = 0)
		: mass(mass), gravRadius(sqrt(G * mass / epsilon)), radius(0), acceleration(glm::vec2(0.0f, 0.0f))
	{
		position = glm::vec2(px, py);
		velocity = glm::vec2(vx, vy);
	}

	// Moves the point of mass
	// -----------------------
	virtual void move()
	{
		accelerate();
		velocity += acceleration;

		// Check for collision
		// glm::vec2 newPosition = position + velocity;
		position += velocity;
	}
	
	// Draws the gravity field
	// -----------------------
	void drawField(const Shader& shader) const
	{
		drawDisk(shader, getGravRadius(), -0.5f);
	}

	// Needed for planets and moons
	virtual void setTerraforming(unsigned int value) {}

	// Getter functions
	glm::vec2 getPosition() const
	{
		return position;
	}
	glm::vec2 getVelocity() const
	{
		return velocity;
	}
	glm::vec2 getAcceleration() const
	{
		return acceleration;
	}
	GLfloat getMass() const
	{
		return mass;
	}
	GLfloat getRadius() const
	{
		return radius;
	}
	GLfloat getGravRadius() const
	{
		return gravRadius;
	}
	virtual std::string getType() const
	{
		return "PointMass";
	}

};


// The main object apart from the player
// -------------------------------------
class Planet : public PointMass
{
protected:
	const glm::vec3 color;
	GLuint terraforming = 0;

public:
	// Constructor
	// -----------
	Planet(const GLfloat mass, const GLfloat radius, const GLfloat r, const GLfloat g, const GLfloat b, const GLfloat px, const GLfloat py, const GLfloat vx = 0, const GLfloat vy = 0)
		: PointMass(mass, px, py, vx, vy), color(glm::vec3(r, g, b))
	{
		this->radius = radius;
	};

	// Draws the planet
	// ----------------
	void draw(const Shader& shader) const
	{
		shader.use();
		shader.setVec3("color", color);
		drawDisk(shader, radius);
	}

	// Draws the atmosphere
	// --------------------
	void drawAtmosphere(const Shader& shader) const
	{
		shader.use();
		shader.setVec3("color", glm::vec3(0.0f, 0.0f, 1.0f));
		drawDisk(shader, radius * atmosphereScale * terraforming / 100, 0.5f);
	}

	void accelerate()
	{
		if (terraforming > 0 && terraforming < 100)
			++terraforming;
	}

	void setTerraforming(const unsigned int value)
	{
		if (!terraforming)
			terraforming = value;
	}

	// Getter functions
	float getRadius() const
	{
		return radius;
	}
	GLuint getTerraforming() const
	{
		return terraforming;
	}
	std::string getType() const
	{
		return "Planet";
	}
};


// A moon orbiting a planet or similar reference object
// ----------------------------------------------------
class Moon : public Planet
{
private:
	const Planet& refPlanet;

public:
	// Constructor
	// -----------
	Moon(const GLfloat mass, const GLfloat radius, const GLfloat r, const GLfloat g, const GLfloat b, const Planet& refPlanet, const GLfloat distance, const GLfloat angle, const bool clockwise)
		: Planet(mass, radius, r, g, b, 0, 0), refPlanet(refPlanet)
	{
		if (refPlanet.getMass() < mass * 2)
		{
			std::cout << "Error: Moon's mass is too large" << std::endl;
		}

		position = refPlanet.getPosition() + distance * glm::vec2(cos(angle), sin(angle));

		// Check objects for intersections

		// Find velocity:
		// Find orthogonal unit vector, reverse if !clockwise, find v = sqrt(GM/r)
		const GLfloat orthogonalAngle = clockwise ? angle - halfPi : angle + halfPi;
		velocity = (GLfloat)sqrt(G * refPlanet.getMass() / distance) * glm::vec2(cos(orthogonalAngle), sin(orthogonalAngle));
	}

	void accelerate()
	{
		acceleration = gravitationalAcceleration(refPlanet) + centrifugalAcceleration(refPlanet);

		if (terraforming > 0 && terraforming < 100)
			++terraforming;
	}
	
	std::string getType() const
	{
		return "Moon";
	}
};


// Same properties as a planet but can't be terraformed, needs 2 shaders and "eats" the space ship upon collision
// --------------------------------------------------------------------------------------------------------------
class BlackHole : public Planet
{
private:
	void accelerate() {}

public:
	BlackHole(const GLfloat mass, const GLfloat px, const GLfloat py, const GLfloat vx = 0, const GLfloat vy = 0)
		: Planet(mass, schwartzschild*mass, 0, 0, 0, px, py, vx, vy)
	{}


	void draw(const Shader& shaderHole, const Shader& shaderHorizon)
	{
		shaderHorizon.use();
		shaderHorizon.setVec3("color", glm::vec3(1.0f));
		drawDisk(shaderHorizon, radius * 1.2f, 0.4f);
		
		shaderHole.use();
		shaderHole.setVec3("color", color);
		drawDisk(shaderHole, radius * 1.1f, 0.6f);
	}
};


// The player model and its movement functions
// -------------------------------------------
class SpaceShip : public PointMass
{
private:
	Planet * startPlanet;
	GLfloat axis;
	GLfloat angle;
	GLfloat launchAngle;
	GLfloat launchSpeed;
	unsigned int launchState = 0;	// 0 not launched, 1 launching, 2 launched, 3 boosted, 4 landed
	bool boosted = false;

public:
	SpaceShip(Planet& startPlanet, GLfloat angle = 90.0f)
		: PointMass(0, 0, 0), startPlanet(&startPlanet), angle(glm::radians(angle)), launchAngle(glm::radians(angle)), launchSpeed(2.0f), axis(startPlanet.getRadius() + spaceShipSize)
	{
		// Find initial position
		GLfloat posX = startPlanet.getPosition().x + (GLfloat)cos(angle) * axis;
		GLfloat posY = startPlanet.getPosition().y + (GLfloat)sin(angle) * axis;
		position = glm::vec2(posX, posY);

		// Find minimal launch speed to exit gravity pull
		// launchSpeed = ceil(glm::length(gravitationalAcceleration(startPlanet)) * 50);
	}

	void launchProgress(const bool boost = false)
	{
		switch (launchState)
		{
		case 0:
			++launchState;
			break;
		case 1:
			std::cout << "Tried to progress launch state during launch" << std::endl;
			break;
		case 2:
			velocity.x += boostPower * (GLfloat)cos(angle);
			velocity.y += boostPower * (GLfloat)sin(angle);
			boosted = true;
			++launchState;
			break;
		case 3:
			std::cout << "Boost already used" << std::endl;
			break;
		case 4:
			std::cout << "Space ship already landed" << std::endl;
			break;
		default:
			std::cout << "launchProgress(): Invalid launch state" << std::endl;
		}

	}

	void accelerate(std::vector<PointMass*> pointMasses)
	{
		acceleration = glm::vec2(0.0f, 0.0f);

		for (PointMass* pm : pointMasses)
		{
			acceleration += gravitationalAcceleration(*pm);
			//acceleration += centrifugalAcceleration(*pm);
		}
	}

	void move(std::vector<PointMass*> pointMasses)
	{
		GLfloat posX, posY;

		switch (launchState)
		{
		case 0:
			angle = launchAngle;
			velocity = glm::vec2(0.0f, 0.0f);
			posX = startPlanet->getPosition().x + (GLfloat)cos(angle) * axis;
			posY = startPlanet->getPosition().y + (GLfloat)sin(angle) * axis;
			position = glm::vec2(posX, posY);
			break;

		case 1:
			velocity.x = launchSpeed * (GLfloat)cos(angle);
			velocity.y = launchSpeed * (GLfloat)sin(angle);
			accelerate(pointMasses);
			velocity += acceleration;
			position += velocity;
			++launchState;
			break;

		case 4:
			break;
		
		// case 2 and 3
		default:
			accelerate(pointMasses);
			velocity += acceleration;
			angle = atan2(velocity.y, velocity.x);	// rotation
			position += velocity;

			// Check for collision
			for (PointMass* pm : pointMasses)
			{
				if (glm::distance(position, pm->getPosition()) - pm->getRadius() <= collisionShip)
				{
					launchState = 4;
				}
			}
		}
	}

	void rotate(bool clockwise, bool precisionMode)
	{
		const GLfloat precisionScale = precisionMode ? 0.1f : 1.0f;

		if (launchState == 0)
		{
			if (clockwise)
				angle -= rotationSpeed * precisionScale;
			else
				angle += rotationSpeed * precisionScale;
		}
		if (angle < 0)
			angle += twicePi;
		else if (angle >= twicePi)
			angle -= twicePi;

		if (launchState == 0)
			launchAngle = angle;
	}

	void adjustSpeed(bool increase, bool precisionMode)
	{
		const GLfloat precisionScale = precisionMode ? 0.05f : 0.5f;

		if (launchState == 0)
		{
			if (increase)
				launchSpeed += precisionScale;
			else
			{
				launchSpeed -= precisionScale;
			}
		}
		if (launchSpeed < 1)
			launchSpeed = 1;
		else if (launchSpeed > 4.0)
			launchSpeed = 4.0;
	}


	void draw(const Shader& shader) const
	{
		// Getting the vertices of the disk
		GLfloat * vertices = getSpaceShip();

		// Generating and binding buffers
		GLuint VBO, VAO;
		glGenBuffers(1, &VBO);

		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 12, vertices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		// Loading the shader and transforming the disk
		shader.use();
		shader.setVec3("color", glm::vec3(200.0f, 200.0f, 200.0f));

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(position.x, position.y, 0.5f));
		model = glm::rotate(model, angle, glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::scale(model, glm::vec3(spaceShipSize, spaceShipSize, 0.0f));
		shader.setMat4("model", model);

		// Drawing the disk
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 12);

		// Clearing the buffers to avoid memory leak
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
	}


	void setPlanet(Planet& newPlanet, const bool reset = false)
	{
		this->startPlanet = &newPlanet;
		axis = startPlanet->getRadius() + spaceShipSize;
		launchState = 0;
		boosted = false;

		GLfloat posX = startPlanet->getPosition().x + (GLfloat)cos(angle) * axis;
		GLfloat posY = startPlanet->getPosition().y + (GLfloat)sin(angle) * axis;
		position = glm::vec2(posX, posY);
		if (not reset)
			launchSpeed = ceil(glm::length(gravitationalAcceleration(*startPlanet)) * 50);
	}

	unsigned int getLaunchState() const
	{
		return launchState;
	}
	GLfloat getAngle() const
	{
		return angle;
	}
	GLfloat getLaunchAngle() const
	{
		return launchAngle;
	}
	GLfloat getLaunchSpeed() const
	{
		return launchSpeed;
	}
	bool hasBoosted() const
	{
		return boosted;
	}
};


// Dummy object that mimics the spaceship's behavior to draw its trajectory
// ------------------------------------------------------------------------
class Trajectory : public PointMass
{
private:
	const SpaceShip& player;
	std::vector<PointMass*> pointMasses;
	std::vector<GLfloat> samples;
	const unsigned int TTL;

	void accelerate()
	{
		acceleration = glm::vec2(0.0f, 0.0f);

		for (auto & pm : pointMasses)
			acceleration += gravitationalAcceleration(*pm);
	}

	void move()
	{
		for (unsigned int i = 0; i < TTL || (i >= TTL && samples.size() % 4 == 0); ++i)
		{
			accelerate();
			velocity += acceleration;
			position += velocity;

			if (i % 10 == 0)
			{
				samples.push_back(position.x);
				samples.push_back(position.y);
			}

			// Check for collision
			for (PointMass* pm : pointMasses)
			{
				if (glm::distance(position, pm->getPosition()) - pm->getRadius() <= collisionShip)
				{
					samples.push_back(position.x);
					samples.push_back(position.y);

					if (samples.size() % 4 == 2)
						return;
				}
			}
		}
	}

public:
	Trajectory(const SpaceShip& player, const std::vector<PointMass*> pointMasses, const unsigned int TTL)
		: PointMass(0, 0, 0), player(player), pointMasses(pointMasses), TTL(TTL)
	{
		update();
	}

	void update()
	{
		samples.clear();

		position = player.getPosition();

		if (player.getLaunchState() > 0)
			velocity = player.getVelocity();
		else
		{
			velocity.x = player.getLaunchSpeed() * (GLfloat)cos(player.getAngle());
			velocity.y = player.getLaunchSpeed() * (GLfloat)sin(player.getAngle());
		}
		
		move();
	}


	void draw(Shader shader)
	{
		GLuint VBO, VAO;
		glGenBuffers(1, &VBO);

		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * samples.size(), &samples.front(), GL_STATIC_DRAW);

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void*)(2*sizeof(float)));
		glEnableVertexAttribArray(0);

		shader.use();
		shader.setVec3("color", glm::vec3(114.0f, 191.0f, 68.0f));

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, -0.25f));
		shader.setMat4("model", model);

		glBindVertexArray(VAO);
		glDrawArrays(GL_LINES, 0, samples.size());

		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
	}

	void setPointMassPointer(const std::vector<PointMass*> pointMasses)
	{
		this->pointMasses = pointMasses;
	}
};

// Terraforming box to be dropped by the player
// --------------------------------------------
class Box : public PointMass
{
private:
	const std::vector<PointMass*> pointMasses;
	GLfloat rotation = 0;
	bool landed = false;
	bool processed = false;
	glm::vec2 restDirection;

	void accelerate(std::vector<PointMass*> pointMasses)
	{
		acceleration = glm::vec2(0.0f, 0.0f);
		for (PointMass* pm : pointMasses)
			acceleration += gravitationalAcceleration(*pm);
	}


public:
	Box(const SpaceShip& player, const std::vector<PointMass*> pointMasses)
		: PointMass(0, player.getPosition().x, player.getPosition().y), pointMasses(pointMasses), restDirection(-player.getVelocity())
	{}

	void accelerate()
	{
		acceleration = glm::vec2(0.0f, 0.0f);
		for (PointMass* pm : pointMasses)
			acceleration += gravitationalAcceleration(*pm);
	}

	void move()
	{
		if (landed)
			return;

		accelerate();
		const GLfloat angle = glm::length(velocity) == 0 ? glm::dot(acceleration, restDirection) : glm::dot(acceleration, velocity);
		rotation += angle;
		velocity += acceleration;

		glm::vec2 newPosition = position + velocity;
		// Check for collision
		for (PointMass* pm : pointMasses)
		{
			if (glm::distance(newPosition, pm->getPosition()) - pm->getRadius() <= collisionBox)
			{
				landed = true;
				pm->setTerraforming(1);
			}
		}

		position += velocity;
	}


	void draw(const Shader& shader) const
	{
		if (landed)
			return;

		// Getting the vertices of the box
		GLfloat * vertices = getBox();

		// Generating and binding buffers
		GLuint VBO, VAO;
		glGenBuffers(1, &VBO);

		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 12, vertices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void*)0);
		glEnableVertexAttribArray(0);

		// Loading the shader and transforming the disk
		shader.use();
		shader.setVec3("color", glm::vec3(152.0f, 80.0f, 6.0f));

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(position.x, position.y, 0.5f));
		model = glm::rotate(model, rotation, glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::scale(model, glm::vec3(boxSize, boxSize, 0.0f));
		shader.setMat4("model", model);

		// Drawing the disk
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 12);

		// Clearing the buffers to avoid memory leak
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
	}

	void process()
	{
		processed = true;
	}

	bool hasLanded() const
	{
		return landed;
	}
	bool isProcessed() const
	{
		return processed;
	}
};


// A pole with a rectangle flag marking the goal planet
// ----------------------------------------------------
class Flag
{
private:
	Planet * goal;
	glm::vec2 position;
	GLfloat time;

public:
	Flag(Planet& goal)
		: goal(&goal), position(goal.getPosition()), time(-(GLfloat)glfwGetTime() * 0.1f + 1.6f)
	{}

	void setPlanet(Planet& newPlanet)
	{
		this->goal = &newPlanet;
		position = goal->getPosition();
	}

	void move()
	{
		position = goal->getPosition();
		time = -(GLfloat)glfwGetTime() * 0.1f + 1.6f;
	}

	void draw(const Shader& shader)
	{
		shader.use();

		// Getting the vertices of the flag
		GLfloat * pole = getFlagPole();
		GLfloat * flag = getFlag();

		// Generating and binding buffers
		GLuint VBO[2], VAO[2];
		glGenBuffers(2, VBO);
		glGenVertexArrays(2, VAO);

		// Pole
		glBindVertexArray(VAO[0]);
		glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 12, pole, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		// Flag
		glBindVertexArray(VAO[1]);
		glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 12, flag, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		// Loading the shader and transforming the pole
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(position.x + (flagSize + goal->getRadius()) * cos(time), position.y + (flagSize + goal->getRadius()) * sin(time), 0.0f));
		model = glm::rotate(model, time + halfPi, glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::scale(model, glm::vec3(flagSize, flagSize, 0.0f));
		shader.setMat4("model", model);
		shader.setVec3("color", glm::vec3(178.0f, 178.0f, 178.0f));

		// Drawing the pole
		glBindVertexArray(VAO[0]);
		glDrawArrays(GL_TRIANGLES, 0, 12);

		// Transforming the flag
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(position.x + (flagSize * 0.9f + goal->getRadius()) * cos(time), position.y + (flagSize + 0.3f + goal->getRadius()) * sin(time), 0.0f));
		model = glm::rotate(model, time - halfPi, glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::scale(model, glm::vec3(flagSize, flagSize, 0.0f));
		shader.setMat4("model", model);
		shader.setVec3("color", glm::vec3(239.0f, 35.0f, 31.0f));

		// Drawing the flag
		glBindVertexArray(VAO[1]);
		glDrawArrays(GL_TRIANGLES, 0, 12);

		// Clearing the buffers to avoid memory leak
		glDeleteVertexArrays(2, VAO);
		glDeleteBuffers(2, VBO);
	}
};


// Yellow disk showing the center of mass of the current planetary system
// ----------------------------------------------------------------------
class CenterOfMass : public PointMass
{
public:
	CenterOfMass() : PointMass(0, 0, 0)	{}

	// Find center of mass for given point masses
	void update(const std::vector<PointMass*>& pointMasses)
	{
		int M = 0;
		for(auto &pm : pointMasses)
		{
			M += (int)pm->getMass();
			position += pm->getMass() * pm->getPosition();
		}
		position /= M;
	}

	void draw(const Shader& shader) const
	{
		shader.use();
		shader.setVec3("color", glm::vec3(255.0f, 255.0f, 0.0f));
		drawDisk(shader, 10, 0.7f);
	}
};


// Oscillating discs in the background
// -----------------------------------
class Star
{
private:
	const glm::vec2 position;
	const glm::vec2 offset;
	const GLfloat radius;
	const GLfloat brightness;

public:
	Star(const glm::vec2 position, const glm::vec2 offset)
		: position(position + 100.0f*offset), radius(glm::length(offset)), brightness((offset.x + offset.y + 2.0f) / 4.0f), offset(offset)
	{}

	glm::vec2 getPosition() const
	{
		return position;
	}
	glm::vec2 getOffset() const
	{
		return offset;
	}
	GLfloat getRadius() const
	{
		return radius;
	}
	GLfloat getBrightness() const
	{
		return brightness;
	}
};

// Draws a gradient rectangle showing the absolute gravitional force
class GravGradient
{
private:
	std::vector<GLfloat> vertices;

	// Calculating the gravitational force applied by another point mass
	glm::vec2 gravitationalAcceleration(const glm::vec2 position, PointMass& other) const
	{
		glm::vec2 rv = other.getPosition() - position;		// Distance vector pointing to the other mass
		GLfloat rl = glm::length(rv);						// Length of distance vector
		return G * other.getMass() / (rl * rl * rl) * rv;
	}

public:
	// Calculates the force in each points and generates the vertices vector
	void update(const GLfloat scrWidth, const GLfloat scrHeight, GLuint xCount, GLuint yCount, std::vector<PointMass*>& pointMasses)
	{
		vertices.clear();

		if (xCount == 0)
			xCount = 2;
		else if (xCount % 2 == 1)
			++xCount;
		if (yCount == 0)
			yCount = 2;
		else if (yCount % 2 == 1)
			++yCount;

		const GLfloat xOffset = scrWidth / (GLfloat)(xCount-1);
		const GLfloat yOffset = scrHeight / (GLfloat)(yCount-1);
		
		std::vector<glm::vec2> positions;
		std::vector<GLfloat> forces;

		glm::vec2 position;
		glm::vec2 force;

		bool insidePlanet;
		GLfloat lastForce;	// Applied when position is inside planet for smoother transitions

		// Get the absolute gravitional force for each point
		for (GLfloat x = 0.0f; x < scrWidth + 0.5f*xOffset; x += xOffset)
		{
			for (GLfloat y = 0.0f; y < scrHeight + 0.5f*yOffset; y += yOffset)
			{
				position = glm::vec2(x, y);
				force = glm::vec2(0.0f);
				insidePlanet = false;

				for (auto& pm : pointMasses)
				{
					if (glm::distance(position, pm->getPosition()) - pm->getRadius() < epsilon)
					{
						insidePlanet = true;
						force = glm::vec2(sqrt(lastForce));
						break;
					}

					force += gravitationalAcceleration(position, *pm);
				}

				if (!insidePlanet)
					lastForce = glm::length(force);

				positions.push_back(position);
				if (glm::length(force) > 0.03f)	// Cap the max force to 0.03f to even out huge differences
					forces.push_back(0.03f);
				else
					forces.push_back(glm::length(force));
			}
		}

		// Find normalizing function [minForce, maxForce] -> [-1, 1]: 2.0f * (slope * x + yIntercept)
		const GLfloat minForce = *std::min_element(forces.begin(), forces.end());
		const GLfloat slope = 1.0f / (*std::max_element(forces.begin(), forces.end()) - minForce);	// Interval of length 1 with slope = 1/(maxForce-minForce)
		const GLfloat yIntercept = -(slope * minForce) - 0.5f;										// Move interval to [-0.5, 0.5]

		// Normalize the forces vector
		std::transform(forces.begin(), forces.end(), forces.begin(), [&slope, &yIntercept](GLfloat x) { return 2.0f * (slope * x + yIntercept); });
		
		/*
		for (const auto& i : forces)
			std::cout << i << std::endl;
		std::cout << std::endl;
		*/

		// Generate the vertices vector
		int currentSamples [6];

		for (unsigned int x = 0; x < xCount-1; ++x)
		{
			for (unsigned int y = 0; y < yCount-1; ++y)
			{
				currentSamples[0] = y + yCount * x;				// Bottom left
				currentSamples[1] = (y + 1) + yCount * x;		// Top left
				currentSamples[2] = (y + 1) + yCount * (x + 1);	// Top right
				currentSamples[3] = y + yCount * x;				// Bottom left
				currentSamples[4] = y + yCount * (x + 1);		// Bottom right
				currentSamples[5] = (y + 1) + yCount * (x + 1);	// Top right

				for (auto cs : currentSamples)
				{
					vertices.push_back(positions[cs].x);	// xPos
					vertices.push_back(positions[cs].y);	// yPos
					vertices.push_back(1.0f + forces[cs]);	// red
					vertices.push_back(1.0f - forces[cs]);	// green
				}	
			}
		}
	}

	// Draws the gradient
	void draw(const Shader& shader) const
	{
		GLuint VBO, VAO;
		glGenBuffers(1, &VBO);

		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vertices.size(), &vertices.front(), GL_STATIC_DRAW);

		// Position attribute
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)0);
		glEnableVertexAttribArray(0);

		// Color attribute
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)(2 * sizeof(float)));
		glEnableVertexAttribArray(1);

		shader.use();
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.8f));
		shader.setMat4("model", model);

		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 4);

		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
	}
};

#endif