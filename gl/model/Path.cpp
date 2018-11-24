//File: Path.cpp
//Brief: Draws an ordered set of points with potentially different colors using opengl.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//glm includes
#include <glm/glm.hpp>

//model includes
#include "gl/model/Path.h"

//c++ includes
#include <iostream>

namespace mygl
{
  Path::Path(VAO::model& vao, const glm::mat4& model, const std::vector<Vertex>& points, const float width): Drawable(model), fNVertices(points.size()+2), fWidth(width)
  {
    Init(vao, points);
  }

  Path::Path(VAO::model& vao, const glm::mat4& model, const std::vector<glm::vec3>& points, const glm::vec4& color, 
             const float width): Drawable(model), fNVertices(points.size()+2), fWidth(width)
  {
    std::vector<Vertex> vertices;
    for(const auto& point: points)
    {
      Vertex vert;
      vert.color = color;
      vert.position = point;
      vertices.push_back(vert);
    }
    Init(vao, vertices);
  }

  void Path::DoDraw(ShaderProg& shader)
  {
    shader.SetUniform("width", fWidth);

    glDrawArrays(GL_LINE_STRIP_ADJACENCY, fOffset, fNVertices);
  }

  void Path::Init(VAO::model& vao, std::vector<Vertex> points)
  {
    //Add one extra vertex on each end of points to provide adjacency information
    points.insert(points.begin(), points.front());
    points.push_back(points.back());

    fOffset = vao.Register(points);
  }

  Path::~Path()
  {
  }
}
