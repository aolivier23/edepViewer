//File: Grid.h
//Brief: A Grid is a Drawable that renders a grid of lines in the x-y plane given the grid's horizontal and vertical 
//       size.  A Grid is rendered by drawing a line between the same two points shifted by a specified spacing in the 
//       horizontal and vertical directions.  
//Author: Andrew Olivier aolivier@ur.rochester.edu

//model includes
#include "gl/model/Drawable.h"
#include "gl/objects/ShaderProg.h"

#ifndef MYGL_GRID_H
#define MYGL_GRID_H

namespace mygl
{
  class Grid: public Drawable //A Grid is a Drawable
  {
    public:
      Grid(const glm::mat4& model, const double width, const double horizSpace, const double height, const double vertSpace, 
           const glm::vec4& color, const float lineWidth);
      virtual ~Grid();

      virtual void DoDraw(mygl::ShaderProg& shader);

    private:
      GLuint fVertVAO; //Location of vertex array object from opengl. 
      GLuint fVertVBO; //Location of vertex buffer object from opengl.
      //Exactly 2 vertices in each VAO
 
      GLuint fHorizVAO; 
      GLuint fHorizVBO;

      const glm::vec4 fColor; //The color of this grid, including transparency
      const double fWidth; //Width of the grid
      const double fHorizSpace; //Spacing between lines in the horizontal (x) direction
      const double fHeight; //Height of the grid
      const double fVertSpace; //Spacing between lines in the vertical (y) direction

      void DrawVertLine(mygl::ShaderProg& prog, const double ypos); //Draw a vertical line at a point pos
      void DrawHorizLine(mygl::ShaderProg& prog, const double xpos); //Draw a horizontal line at a point pos

      const float fLineWidth; //The width of lines to draw in NDC coordinates
  };
}

#endif //MYGL_GRID_H
