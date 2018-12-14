//File: Grid.cpp
//Brief: Draws an ordered set of points with potentially different colors using opengl.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//glm includes
#include <glm/glm.hpp>

//model includes
#include "gl/model/Grid.h"

namespace mygl
{
  Grid::Grid(VAO::model& vao, const glm::mat4& model, const double width, const double horizSpace, const double height, const double vertSpace,
             const glm::vec4& color, const float lineWidth): Drawable(model), fWidth(width), fHorizSpace(horizSpace), 
                                                             fHeight(height), fVertSpace(vertSpace), fLineWidth(lineWidth)
  {
    //Set up vertical line
    std::vector<Vertex> points;
    Vertex vert;
    vert.position = glm::vec3(0.f, -height/2., 0.f);
    vert.color = color;
    points.push_back(vert);
    
    vert.position = glm::vec3(0.f, -height/2., 0.f);
    points.push_back(vert);
   
    vert.position = glm::vec3(0.f, height/2., 0.f);
    points.push_back(vert);
 
    vert.position = glm::vec3(0.f, height/2., 0.f);
    points.push_back(vert);

    //Set up vertex attributes expected by vertex shader
    vert.position = glm::vec3(-width/2., 0.f, 0.f);
    points.push_back(vert);

    vert.position = glm::vec3(-width/2., 0.f, 0.f);
    points.push_back(vert);

    vert.position = glm::vec3(width/2., 0.f, 0.f);
    points.push_back(vert);

    vert.position = glm::vec3(width/2., 0.f, 0.f);
    points.push_back(vert);

    fOffset = vao.Register(points);
  }

  void Grid::DoDraw(ShaderProg& shader)
  {
    shader.SetUniform("width", fLineWidth);

    //Draw horizontal lines
    for(double ypos = -fHeight/2.; ypos < fHeight/2.; ypos += fVertSpace) DrawHorizLine(shader, ypos);    
    DrawHorizLine(shader, fHeight/2.);

    //Draw vertical lines
    for(double xpos = -fWidth/2.; xpos < fWidth/2.; xpos += fHorizSpace) DrawVertLine(shader, xpos);
    DrawVertLine(shader, fWidth/2.);
  }

  void Grid::DrawVertLine(ShaderProg& shader, const double xpos)
  {
    shader.SetUniform("model", glm::translate(fModel, glm::vec3(xpos, 0.f, 0.f)));
    glDrawArrays(GL_LINE_STRIP_ADJACENCY, fOffset, 4);
  }

  void Grid::DrawHorizLine(ShaderProg& shader, const double ypos)
  {
    shader.SetUniform("model", glm::translate(fModel, glm::vec3(0.f, ypos, 0.f)));
    glDrawArrays(GL_LINE_STRIP_ADJACENCY, fOffset+4, 4);
  }

  Grid::~Grid()
  {
  }
}
