#ifndef GAME_OBJECTS_H
#define GAME_OBJECTS_H

#include <glad/glad.h>	// OpenGL headers
#include "glm/glm.hpp"	// Vectors
#include "shader.hpp"
#include "shapes.hpp"

#include <cmath>

// constants
const float G = 6.6743f;		// Gravitational constant
const float epsilon = 0.1f;		// Minimal gravitational force to visualize

// The core of all physics object
// ------------------------------
class PointMass
{
private:
	const float mass;
	const float gravRadius;
	glm::vec2 position;
	glm::vec2 velocity;
	glm::vec2 acceleration;

	// Calculating the gravitational force applied by another point mass
	// -----------------------------------------------------------------
	glm::vec2 gravitationalAcceleration(PointMass other)
	{
		glm::vec2 rv = other.getPosition() - this->position;	// Distance vector pointing to the other mass
		float rl = glm::length(rv);								// Length of distance vector
		return G * other.getMass() / (rl * rl * rl) * rv;
	}

	// Calculating the centrifugal force applied by another point mass
	// ---------------------------------------------------------------
	glm::vec2 centrifugalAcceleration(PointMass other)
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

		glm::vec2 rv = this->position - other.getPosition();			// Distance vector pointing away from the other mass
		float rl = glm::length(rv);										// Length of the distance vector
		glm::vec2 rv_u = rv / rl;										// Unit vector
		
		glm::vec2 velocity_u = glm::normalize(velocity);				// Velocity unit vector
		float angle = acos(glm::dot(-rv_u, velocity_u));				// Angle between the 2 unit vectors
		float orbitalVelocity = glm::length(velocity) * sin(angle);		// Length of orthogonal component of the velocity vector

		return orbitalVelocity * orbitalVelocity / rl * rv;
	}

public:
	// Constructor
	// Arguments: mass, position.x, position.y
	// Optional: velocity.x, velocity.y
	// ---------------------------------------
	PointMass(float mass, float px, float py, float vx = 0, float vy = 0) : mass(mass), gravRadius(sqrt(G * mass / epsilon))
	{
		position = glm::vec2(px, py);
		velocity = glm::vec2(vx, vy);
	}

	// Draws the gravitional field as a disk
	// -------------------------------------
	void drawField(Shader shader)
	{
		// Setting up the disk
		GLfloat * vertices = getDisk();

		// Drawing the disk
		GLuint VBO, VAO;
		glGenBuffers(1, &VBO);

		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		
		shader.use();
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(position, 0.0f));
		shader.setMat4("model", model);

		glDrawArrays(GL_TRIANGLE_FAN, 0, nSegments);
	}

	// Getter functions
	glm::vec2 getPosition()
	{
		return position;
	}
	glm::vec2 getVelocity()
	{
		return velocity;
	}
	glm::vec2 getAcceleration()
	{
		return acceleration;
	}
	float getMass()
	{
		return mass;
	}
	float getGravRadius()
	{
		return gravRadius;
	}

};

#endif