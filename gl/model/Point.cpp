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
  Point::Point(VAO::model& vao, const glm::mat4& model, const glm::vec3& point, const glm::vec4& color, const float radius): Drawable(model), fNVertices(1), 
                                                                                                                      fRadius(radius)
  {
    Vertex vert;
    vert.position = point;
    vert.color = color;

    fOffset = vao.Register(std::vector<Vertex>(1, vert));
  }

  void Point::DoDraw(ShaderProg& shader)
  {
    shader.SetUniform("radius", fRadius);

    glDrawArrays(GL_POINTS, fOffset, fNVertices);
  }

  Point::~Point()
  {
  }
}
