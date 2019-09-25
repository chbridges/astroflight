#ifndef GUI_H
#define GUI_H

// Text rendering
#include <ft2build.h>
#include FT_FREETYPE_H

#include <glad/glad.h>
#include "glm/glm.hpp"
#include "shader.hpp"

#include <map>

namespace GUI
{
    struct Character
    {
        GLuint textureID;   // glyph texture
        glm::ivec2 size;    // glyph size
        glm::ivec2 bearing; // baseline offset
        GLuint advance;     // horizontal offset to next glyph
    };

    std::map<GLchar, Character> characters;

    void textInit()
    {
        FT_Library ft;
        FT_Face face;

        if (FT_Init_FreeType(&ft))
            std::cout << "ERROR::FREETYPE: Could not init FreeType library " << std::endl;
        
        if (FT_New_Face(ft, "gui/nasaliza.ttf", 0, &face))
            std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;

        FT_Set_Pixel_Sizes(face, 0, 48);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        for (GLubyte c = 0; c < 128; ++c)
        {
            // Load character
            if (FT_Load_Char(face, c, FT_LOAD_RENDER))
            {
                std::cout << "ERROR::FREETYPE: Failed to load glyph" << std::endl;
                continue;
            }

            // Generate texture
            GLuint texture;
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RED,
                face->glyph->bitmap.width,
                face->glyph->bitmap.rows,
                0,
                GL_RED,
                GL_UNSIGNED_BYTE,
                face->glyph->bitmap.buffer
            );
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            // Store character
            Character character = {
                texture,
                glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                (GLuint)face->glyph->advance.x
            };
            characters.insert(std::pair<GLchar, Character>(c, character));
        }
        glBindTexture(GL_TEXTURE_2D, 0);
        FT_Done_Face(face);
        FT_Done_FreeType(ft);
    }

    void renderText(const Shader &shader, const std::string text, GLfloat x, const GLfloat y, const GLfloat scale, const glm::vec3 color)
    {
        shader.use();
        shader.setVec3("textColor", color);

        GLuint VBO, VAO;
		glGenBuffers(1, &VBO);
		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
        glActiveTexture(GL_TEXTURE0);
        
        std::string::const_iterator c;
        for (c = text.begin(); c < text.end(); ++c)
        {
            Character ch = characters[*c];

            GLfloat xpos = x + ch.bearing.x * scale;
            GLfloat ypos = y - (ch.size.y - ch.bearing.y) * scale;
            GLfloat width = ch.size.x * scale;
            GLfloat height = ch.size.y * scale;
            GLfloat vertices[6][4] = {
                { xpos,         ypos + height,  0.0f, 0.0f },
                { xpos,         ypos,           0.0f, 1.0f },
                { xpos + width, ypos,           1.0f, 1.0f },
                { xpos,         ypos + height,  0.0f, 0.0f },
                { xpos + width, ypos,           1.0f, 1.0f },
                { xpos + width, ypos + height,  1.0f, 0.0f }
            };
            glBindTexture(GL_TEXTURE_2D, ch.textureID);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            x += (ch.advance >> 6) * scale;
        }
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);

        glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
    }

    void renderBox(const Shader &shader, const GLfloat x, const GLfloat y, const GLfloat width, const GLfloat height, const glm::vec4 color)
    {
        shader.use();
        shader.setVec4("color", color);

        const GLfloat vertices[12] = {
		    x,          y,
	    	x,          y+height,
    		x+width,    y+height,
		    x,          y,
	    	x+width,    y,
    		x+width,    y+height
	    };

        GLuint VBO, VAO;
		glGenBuffers(1, &VBO);

		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 12, vertices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void*)0);
		glEnableVertexAttribArray(0);

		glm::mat4 model = glm::mat4(1.0f);
		shader.setMat4("model", model);

		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 12);

		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
    }
}

#endif