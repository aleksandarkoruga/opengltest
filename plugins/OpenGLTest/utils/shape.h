/*
Title: Rendering to Textures
File Name: shape.h
Copyright ? 2016
Author: David Erbelding
Written under the supervision of David I. Schwartz, Ph.D., and
supported by a professional development seed grant from the B. Thomas
Golisano College of Computing & Information Sciences
(https://www.rit.edu/gccis) at the Rochester Institute of Technology.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or (at
your option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "GLHeaders.h"

//struct for vertex with uv
struct Vertex2dUVColor {

    glm::vec2 m_position;
    glm::vec2 m_texCoord;
    glm::vec4 m_color;

    // Makes a 2d vertex with uc and color data.
    Vertex2dUVColor(glm::vec2 position, glm::vec2 texCoord, glm::vec4 color) {
        m_position = position;
        m_texCoord = texCoord;
        m_color = color;
    }
};

class Shape {


private:
	// Vectors of shape information
	std::vector<Vertex2dUVColor> m_vertices;
	std::vector<unsigned int> m_indices;

	// Buffered shape info
	GLuint m_vertexBuffer;
	GLuint m_indexBuffer;


public:
	// Constructor for a shape, takes a vector for vertices and indices
	Shape(std::vector<Vertex2dUVColor> vertices, std::vector<unsigned int> indices);
	// Shape destructor to clean up buffers
	~Shape();

	// Draws the shape using a given world matrix
	void Draw(glm::mat3 worldMatrix, GLuint worldMatrixUniform);
};