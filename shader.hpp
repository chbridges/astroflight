#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>	// OpenGL headers
#include "glm/glm.hpp"	// Transformations

#include <string>		// Storing the shader code
#include <fstream>		// Open the shader code file
#include <sstream>		// Reading the file
#include <iostream>		// For error/debug console output


// Compiles and links shader programs with given vertex and fragment shader
// ------------------------------------------------------------------------
class Shader
{
private:
	// Program ID
	// ----------
	GLuint ID;

	// Utility function for checking shader compilation/linking errors
	// Arguments: shader or program ID, type (vertex, fragment, program)
	// -----------------------------------------------------------------
	void checkErrors(GLuint ID, std::string type)
	{
		GLint success;
		GLchar infoLog[1024];

		if (type != "PROGRAM")
		{
			glGetShaderiv(ID, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(ID, 512, NULL, infoLog);
				std::cout << "ERROR::SHADER::" << type <<"::COMPILATION_FAILED\n" << infoLog;
			}
		}
		else
		{
			glGetProgramiv(ID, GL_LINK_STATUS, &success);
			if (!success)
			{
				glGetProgramInfoLog(ID, 512, NULL, infoLog);
				std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog;
			}
		}
	}

public:
	// Constructor: reading compiling and linking shader program
	// Arguments: Vertex shader and fragment shader filenames without the extensions
	// -----------------------------------------------------------------------------
	Shader(const GLchar * vertexFileName, const GLchar * fragmentFileName)
	{
		// 1. Reading the shaders
		// ----------------------
		std::string vertexCode, fragmentCode;	// The shader codes to be read from the disc
		std::ifstream vertexFile, fragmentFile;	// File streams for the vertex/fragment shader code files, respectively

		// Getting the filepath from the arguments
		std::string vertexPath = "shaders/" + std::string(vertexFileName) + ".vsh";
		std::string fragmentPath = "shaders/" + std::string(fragmentFileName) + ".fsh";

		// Reading the vertex shader
		vertexFile.open(vertexPath, std::ios::in);
		if (vertexFile.is_open())
		{
			std::stringstream vertexStream;
			vertexStream << vertexFile.rdbuf();
			vertexCode = vertexStream.str();
			vertexFile.close();
		}
		else
		{
			std::cout << "Could not open " << vertexPath << std::endl;
		}

		// Reading the fragment shader
		fragmentFile.open(fragmentPath, std::ios::in);
		if (fragmentFile.is_open())
		{
			std::stringstream fragmentStream;
			fragmentStream << fragmentFile.rdbuf();
			fragmentCode = fragmentStream.str();
			fragmentFile.close();
		}
		else
		{
			std::cout << "Could not open " << fragmentPath << std::endl;
		}

		const char * vertexSourcePointer = vertexCode.c_str();
		const char * fragmentSourcePointer = fragmentCode.c_str();
		
		// 2. Compiling the shaders
		// ------------------------
		GLuint vertexID, fragmentID;
		
		// Compiling the vertex shader
		vertexID = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexID, 1, &vertexSourcePointer, NULL);
		glCompileShader(vertexID);
		checkErrors(vertexID, vertexFileName);

		// Compiling the fragment shader
		fragmentID = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentID, 1, &fragmentSourcePointer, NULL);
		glCompileShader(fragmentID);
		checkErrors(fragmentID, fragmentFileName);

		// 3. Linking the program
		// ----------------------
		ID = glCreateProgram();
		glAttachShader(ID, vertexID);
		glAttachShader(ID, fragmentID);
		glLinkProgram(ID);
		checkErrors(ID, "PROGRAM");

		// 4. Detaching and deleting the shaders
		// -------------------------------------
		glDetachShader(ID, vertexID);
		glDetachShader(ID, fragmentID);

		glDeleteShader(vertexID);
		glDeleteShader(fragmentID);
	}

	// Using the shader program
	// ------------------------
	void use()
	{
		glUseProgram(ID);
	}

	// Utility uniform functions
	// -------------------------
	void setBool(const std::string &name, bool value) const
	{
		glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
	}
	void setInt(const std::string &name, int value) const
	{
		glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
	}
	void setFloat(const std::string &name, float value) const
	{
		glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
	}

	// Vectors
	void setVec2(const std::string &name, const glm::vec2 &value) const
	{
		glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
	}
	void setVec2(const std::string &name, float x, float y) const
	{
		glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
	}
	void setVec3(const std::string &name, const glm::vec3 &value) const
	{
		glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
	}
	void setVec3(const std::string &name, float x, float y, float z) const
	{
		glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
	}
	void setVec4(const std::string &name, const glm::vec4 &value) const
	{
		glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
	}
	void setVec4(const std::string &name, float x, float y, float z, float w) const
	{
		glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
	}

	// Matrices
	void setMat2(const std::string &name, const glm::mat2 &mat) const
	{
		glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}
	void setMat3(const std::string &name, const glm::mat3 &mat) const
	{
		glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}
	void setMat4(const std::string &name, const glm::mat4 &mat) const
	{
		glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}

};

#endif