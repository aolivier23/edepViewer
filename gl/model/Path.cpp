//File: Path.cpp
//Brief: Draws an ordered set of points with potentially different colors using opengl.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//glm includes
#include <glm/glm.hpp>

//model includes
#include "gl/model/Path.h"
#include "gl/model/ShaderProg.h"

//c++ includes
#include <iostream>

namespace mygl
{
  Path::Path(const glm::mat4& model, const std::vector<Vertex>& points): Drawable(model), fNVertices(points.size())
  {
    Init(points);
  }

  Path::Path(const glm::mat4& model, const std::vector<glm::vec3>& points, const glm::vec4& color): Drawable(model), fNVertices(points.size())
  {
    std::vector<Vertex> vertices;
    for(const auto& point: points)
    {
      Vertex vert;
      vert.color = color;
      vert.position = point;
      vertices.push_back(vert);
    }
    Init(vertices);
  }

  void Path::DoDraw(ShaderProg& shader)
  {
    shader.Use();

    glBindVertexArray(fVAO);
    glDrawArrays(GL_LINE_STRIP, 0, fNVertices); //GL_LINE_STRIP_ADJACENCY, 0, fNVertices);

    glBindVertexArray(0); //Unbind data after done drawing
  }

  void Path::Init(const std::vector<Vertex>& points)
  {
    std::cout << "GLEW version is " << glewGetString(GLEW_VERSION) << "\n";
    std::cout << "OpenGL version is " << glGetString(GL_VERSION) << "\n";

    //Set up vertices for drawing. 
    glGenVertexArrays(1, &fVAO);
    glBindVertexArray(fVAO);

    //Construct buffer for vertices 
    glGenBuffers(1, &fVBO);
    glBindBuffer(GL_ARRAY_BUFFER, fVBO);

    glBufferData(GL_ARRAY_BUFFER, points.size()*sizeof(Vertex), &points[0], GL_STATIC_DRAW);

    //Set up vertex attributes expected by vertex shader
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(0));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)sizeof(glm::vec3));
  }
}
