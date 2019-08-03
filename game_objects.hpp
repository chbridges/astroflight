#ifndef GAME_OBJECTS_H
#define GAME_OBJECTS_H

#include <glad/glad.h>	// OpenGL headers
#include "glm/glm.hpp"	// Vectors and transformation matrices
#include "shader.hpp"
#include "shapes.hpp"

#include <vector>
#include <cmath>

// Constants
const GLfloat scale = 1.0f;				// Scales gravity force and keeps draw distance consistent
const GLfloat G = 6.6743f * scale;		// Scaled gravitational constant
const GLfloat epsilon = 0.01f * scale;	// Minimal gravitational force to visualize
const GLfloat spaceShipSize = 25.0f;
const GLfloat rotationSpeed = 1.0f / 180.0f * pi;
const GLfloat boostPower = 2.0f;
const GLfloat collisionScale = 0.5f;	// Lower value = smaller collision box, negative values possible
const GLfloat boxSize = 5.0f;
const GLfloat boxRotation = 1.0f / 180.0f * pi;
const GLfloat flagSize = 15.0f;

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

	// Calculating the gravitational force applied by another point mass
	// -----------------------------------------------------------------
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
		/*
		 *	The following algorithm calculates the centrifugal force using the compononent of the velocity vector v
		 *	orthogonal to the distance vector u pointing towards the other mass. It has been derived using the formula:
		 *		u*v = |u|*|v|*cos(theta)
		 *	or for normalized vectors:
		 *		u*v = cos(theta)
		 *	For 90° the orbital velocity is equal to the velocity, i.e. scalar = 1 = sin(90°).
		 *	For 0° the orbital velocity is 0, i.e. scalar = 0 = sin(0°).
		 *	The vector representing the centrifugal acceleration therefore has the length
		 *		|v|*|sin(theta)|
		 *    = |v|*|sin(arccos(u*v))|	, u and v being unit vectors
		 *	As the centrifugal force pointing into the opposite direction of the distance vector uses the square of the
		 *	orbital velocity, the absolute value of the sine is neglectable.
		 */

		/*
		glm::vec2 rv = this->position - other.getPosition();			// Distance vector pointing away from the other mass
		GLfloat rl = glm::length(rv);									// Length of the distance vector
		glm::vec2 rv_u = rv / rl;										// Unit vector
		
		glm::vec2 velocity_u = glm::normalize(velocity);				// Velocity unit vector
		GLfloat angle = acos(glm::dot(rv_u, velocity_u));				// Angle between the 2 unit vectors
		GLfloat orbitalVelocity = glm::length(velocity) * sin(angle);	// Length of orthogonal component of the velocity vector

		std::cout << "Sine: " << sin(angle) << " Velocity: " << orbitalVelocity << std::endl;

		return orbitalVelocity * orbitalVelocity / rl * rv;
		*/

		
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
	GLuint terraforming;

public:
	// Constructor
	// -----------
	Planet(const GLfloat mass, const GLfloat radius, const GLfloat r, const GLfloat g, const GLfloat b, const GLfloat px, const GLfloat py, const GLfloat vx = 0, const GLfloat vy = 0)
		: PointMass(mass, px, py, vx, vy), color(glm::vec3(r, g, b)), terraforming(0)
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

	// Getter functions
	float getRadius() const
	{
		return radius;
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
		const GLfloat orthogonalAngle = clockwise ? angle + halfPi : angle - halfPi;
		velocity = (GLfloat)sqrt(G * refPlanet.getMass() / distance) * glm::vec2(cos(orthogonalAngle), sin(orthogonalAngle));
	}

	void accelerate()
	{
		acceleration = gravitationalAcceleration(refPlanet) + centrifugalAcceleration(refPlanet);
	}

	std::string getType() const
	{
		return "Moon";
	}
};


class SpaceShip : public PointMass
{
private:
	Planet * startPlanet;
	GLfloat axis;
	GLfloat angle;
	GLfloat launchAngle;
	GLfloat launchSpeed;
	unsigned int launchState;	// 0 not launched, 1 launching, 2 launched, 3 boosted, 4 landed

public:
	SpaceShip(Planet& startPlanet, GLfloat angle = 90.0f)
		: PointMass(0, 0, 0), startPlanet(&startPlanet), angle(glm::radians(angle)), launchAngle(glm::radians(angle)), launchState(0), axis(startPlanet.getRadius() + spaceShipSize)
	{
		// Find initial position
		GLfloat posX = startPlanet.getPosition().x + (GLfloat)cos(angle) * axis;
		GLfloat posY = startPlanet.getPosition().y + (GLfloat)sin(angle) * axis;
		position = glm::vec2(posX, posY);

		// Find minimal launch speed to exit gravity pull
		launchSpeed = glm::length(gravitationalAcceleration(startPlanet)) * 50;
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

			/*
			/velocity.x += 0.1f * launchSpeed * (GLfloat)cos(angle);
			/velocity.y += 0.1f * launchSpeed * (GLfloat)sin(angle);

			if (glm::length(velocity) >= launchSpeed)
			{
				velocity = glm::normalize(velocity);
				velocity *= launchSpeed;
				++launchState;
			}
			*/

			++launchState;
			position += velocity;
			break;

		case 4:
			break;
		
		// case 2 and 3
		default:
			accelerate(pointMasses);
			velocity += acceleration;
			angle = atan2(velocity.y, velocity.x);	// rotation
			glm::vec2 newPosition = position + velocity;

			// Check for collision
			for (PointMass* pm : pointMasses)
			{
				if (glm::distance(newPosition, pm->getPosition()) - pm->getRadius() <= collisionScale * spaceShipSize * 0.17f)
				{
					launchState = 4;
					return;
				}
			}

			position = newPosition;
		}
	}

	void rotate(bool clockwise, bool precisionMode)
	{
		const GLfloat precisionScale = precisionMode ? 0.5f : 1.0f;

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
		const GLfloat precisionScale = precisionMode ? 0.5f : 1.0f;

		if (launchState == 0)
		{
			if (increase)
				++launchSpeed;
			else
			{
				--launchSpeed;
			}
		}
		if (launchSpeed < 1)
			++launchSpeed;
		else if (launchSpeed > 10)
			--launchSpeed;
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

		GLfloat posX = startPlanet->getPosition().x + (GLfloat)cos(angle) * axis;
		GLfloat posY = startPlanet->getPosition().y + (GLfloat)sin(angle) * axis;
		position = glm::vec2(posX, posY);
		if (not reset)
			launchSpeed = glm::length(gravitationalAcceleration(*startPlanet)) * 50;
	}

	void setLaunchState(const unsigned int newState)
	{
		launchState = newState;
	}

	unsigned int getLaunchState() const
	{
		return launchState;
	}

	GLfloat getAngle() const
	{
		return glm::degrees(angle);
	}

	GLfloat getLaunchSpeed() const
	{
		return launchSpeed;
	}
};


class Box : public PointMass
{
private:
	const std::vector<PointMass*> pointMasses;
	GLfloat rotation;
	bool landed;
	glm::vec2 restDirection;

	void accelerate(std::vector<PointMass*> pointMasses)
	{
		acceleration = glm::vec2(0.0f, 0.0f);
		for (PointMass* pm : pointMasses)
			acceleration += gravitationalAcceleration(*pm);
	}


public:
	Box(const SpaceShip& player, const std::vector<PointMass*> pointMasses)
		: PointMass(0, player.getPosition().x, player.getPosition().y), pointMasses(pointMasses), restDirection(-player.getVelocity()), rotation(0), landed(false)
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
			if (glm::distance(newPosition, pm->getPosition()) - pm->getRadius() <= collisionScale * boxSize * 0.67f)
				landed = true;
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

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
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
};


class Star
{
public:
	Star(const GLfloat scrWidth, const GLfloat scrHeight)
	{
		// randomize position and radius in initializer list
		// use velocity for rotation
	}

	void draw(const Shader& shader)
	{
		shader.use();
		// use sine to vary brightness & size, draw disk with gradient shader
	}
};


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
		model = glm::translate(model, glm::vec3(position.x + (flagSize + goal->getRadius()) * cos(time), position.y + (flagSize + goal->getRadius()) * sin(time), 0.5f));
		model = glm::rotate(model, time + halfPi, glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::scale(model, glm::vec3(flagSize, flagSize, 0.0f));
		shader.setMat4("model", model);
		shader.setVec3("color", glm::vec3(178.0f, 178.0f, 178.0f));

		// Drawing the pole
		glBindVertexArray(VAO[0]);
		glDrawArrays(GL_TRIANGLES, 0, 12);

		// Transforming the flag
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(position.x + (flagSize * 1.1f + goal->getRadius()) * cos(time), position.y + (flagSize + 0.3f + goal->getRadius()) * sin(time), 0.5f));
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

#endif