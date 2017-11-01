//File: Grid.cpp
//Brief: Draws an ordered set of points with potentially different colors using opengl.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//glm includes
#include <glm/glm.hpp>

//model includes
#include "gl/model/Grid.h"
#include "gl/model/ShaderProg.h"

namespace mygl
{
  Grid::Grid(const glm::mat4& model, const double width, const double horizSpace, const double height, const double vertSpace,
       const glm::vec4& color): Drawable(model), fColor(color), fWidth(width), fHorizSpace(horizSpace), fHeight(height), 
                                fVertSpace(vertSpace)
  {
    //Set up vertical line
    //Set up vertices for drawing.
    glGenVertexArrays(1, &fVertVAO);
    glBindVertexArray(fVertVAO);
    
    //Construct buffer for vertices 
    glGenBuffers(1, &fVertVBO);
    glBindBuffer(GL_ARRAY_BUFFER, fVertVBO);

    glm::vec3 points[] = {glm::vec3(0.f, -height/2., 0.f), glm::vec3(0.f, height/2., 0.f)};
    glBufferData(GL_ARRAY_BUFFER, 2*sizeof(glm::vec3), points, GL_STATIC_DRAW);

    //Set up vertex attributes expected by vertex shader
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (GLvoid*)(0));

    //Set up horizontal line
    //Set up vertices for drawing.
    glGenVertexArrays(1, &fHorizVAO);
    glBindVertexArray(fHorizVAO);
    
    //Construct buffer for vertices 
    glGenBuffers(1, &fHorizVBO);
    glBindBuffer(GL_ARRAY_BUFFER, fHorizVBO);

    points[0] = glm::vec3(-width/2., 0.f, 0.f);
    points[1] = glm::vec3(width/2., 0.f, 0.f);

    glBufferData(GL_ARRAY_BUFFER, 2*sizeof(glm::vec3), points, GL_STATIC_DRAW);
    
    //Set up vertex attributes expected by vertex shader
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (GLvoid*)(0));
  }

  void Grid::DoDraw(ShaderProg& shader)
  {
    shader.Use();
    shader.SetUniform("userColor", fColor.r, fColor.g, fColor.b, fColor.a);

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
    glBindVertexArray(fVertVAO);
    glDrawArrays(GL_LINE_STRIP, 0, 2);
    glBindVertexArray(0); //Unbind data after done drawing
  }

  void Grid::DrawHorizLine(ShaderProg& shader, const double ypos)
  {
    shader.SetUniform("model", glm::translate(fModel, glm::vec3(0.f, ypos, 0.f)));
    glBindVertexArray(fHorizVAO);
    glDrawArrays(GL_LINE_STRIP, 0, 2);
    glBindVertexArray(0); //Unbind data after done drawing
  }
}
