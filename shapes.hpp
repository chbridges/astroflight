#ifndef SHAPES_H
#define SHAPES_H

#include <glad/glad.h>	// OpenGL headers
#include "glm/glm.hpp"
#include <cmath>
#include <iostream>


// constants
const float twicePi = 2 * 3.141592653589793f;
const int nSegments = 100;		// Number of circle segments
const int nVertices = nSegments + 2;

GLfloat * getDisk()
{
	static GLfloat vertices[nVertices * 2] = { 0.0f };	// initialize triangle fan with center in (0,0)

	for (int i = 1; i < nVertices; ++i)		// start loop at second vertice
	{
		vertices[2 * i] = cos(i * twicePi / nSegments);
		vertices[2 * i + 1] = sin(i * twicePi / nSegments);
		//std::cout << i << " " << vertices[2 * i] << " " << vertices[2 * i + 1] << std::endl;
	}

	return vertices;
}

#endif