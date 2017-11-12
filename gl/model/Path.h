//File: Path.h
//Brief: A path is an ordered series of points.  A Path object is a set of instructions 
//       for mygl::Viewer to render a path in opengl by connecting the points in order. 
//       Path objects can be constructed with a single color or a collection of 7-vectors 
//       whose last four elements are the red, greeen, blue, and alpha components to use 
//       when drawing that segment.  Opengl will blend the colors of adjacent points together 
//       to color the line between them.  
//Author: Andrew Olivier aolivier@ur.rochester.edu

//model includes
#include "gl/model/Drawable.h"
#include "gl/model/ShaderProg.h"

//c++ includes
#include <vector>

#ifndef MYGL_PATH_H
#define MYGL_PATH_H

namespace mygl
{
  class Path: public Drawable //A Path is a Drawable
  {
    public:
      struct Vertex
      {
        glm::vec3 position;
        glm::vec4 color;
      };

      Path(const glm::mat4& model, const std::vector<Vertex>& points);
      Path(const glm::mat4& model, const std::vector<glm::vec3>& points, const glm::vec4& color);
      virtual ~Path() = default;

      virtual void DoDraw(mygl::ShaderProg& shader);

    private:
      GLuint fVAO; //Location of vertex array object from opengl. 
      GLuint fVBO; //Location of vertex buffer object from opengl.
      const GLuint fNVertices; //Number of vertices in this path

      void Init(std::vector<Vertex> points);
  };
}

#endif //MYGL_PATH_H
