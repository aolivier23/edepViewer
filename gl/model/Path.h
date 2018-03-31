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
#include "gl/objects/ShaderProg.h"

//c++ includes
#include <vector>

#ifndef MYGL_PATH_H
#define MYGL_PATH_H

namespace mygl
{
  class VAO;

  class Path: public Drawable //A Path is a Drawable
  {
    public:
      Path(VAO& vao, const glm::mat4& model, const std::vector<Vertex>& points, const float width);
      Path(VAO& vao, const glm::mat4& model, const std::vector<glm::vec3>& points, const glm::vec4& color, const float width);
      virtual ~Path();

      virtual void DoDraw(mygl::ShaderProg& shader);

    private:
      const GLuint fNVertices; //Number of vertices in this path

      void Init(VAO& vao, std::vector<Vertex> points);

      const float fWidth;
  };
}

#endif //MYGL_PATH_H
