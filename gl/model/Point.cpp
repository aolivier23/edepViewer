//File: Point.cpp
//Brief: Draws a point.  With no fancy shaders, will just be one pixel wide.  fRadius is intended to be used 
//       with a geometry (or tesselation) shader to produce a wider point.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//glm includes
#include <glm/glm.hpp>

//model includes
#include "gl/model/Point.h"

//c++ includes
#include <iostream>

namespace mygl
{
  Point::Point(const glm::mat4& model, const glm::vec3& point, const glm::vec4& color, const float radius): Drawable(model), fNVertices(1), 
                                                                                                            fRadius(radius)
  {
    //TODO: Do I really need a VAO and a VBO for a single point?  This really points to the need for 
    //      a VAO sharing model within Scene.  
    //Set up vertices for drawing. 
    glGenVertexArrays(1, &fVAO);
    glBindVertexArray(fVAO);

    //Construct buffer for vertices 
    glGenBuffers(1, &fVBO);
    glBindBuffer(GL_ARRAY_BUFFER, fVBO);

    Vertex vert;
    vert.position = point;
    vert.color = color;

    glBufferData(GL_ARRAY_BUFFER, fNVertices*sizeof(Vertex), &vert, GL_STATIC_DRAW);

    //Set up vertex attributes expected by vertex shader
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(0));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)sizeof(glm::vec3));

    glBindVertexArray(0); //Unbind to prevent mistakes writing to the wrong buffer
  }

  void Point::DoDraw(ShaderProg& shader)
  {
    shader.SetUniform("radius", fRadius);

    glBindVertexArray(fVAO);
    glDrawArrays(GL_POINTS, 0, fNVertices);

    glBindVertexArray(0); //Unbind data after done drawing
  }

  Point::~Point()
  {
    glDeleteVertexArrays(1, &fVAO);
    glDeleteBuffers(1, &fVBO);
  }
}
