//File: Point.h
//Brief: A Point draws a single circle on the screen.  This class itself describes opengl's point primitive, which is only 
//       one pixel wide by default.  Use with a geometry or tesselation shader (or with glPointSize) to get more usable points.  
//Author: Andrew Olivier aolivier@ur.rochester.edu
//TODO: A PointSet might be much more powerful, but I haven't thought of a way to make individual points selectable with a 
//      PointSet when using my current Scene->Viewer class hierarchy.  

//model includes
#include "gl/model/Drawable.h"
#include "gl/objects/ShaderProg.h"
#include "gl/objects/VAO.h"

//c++ includes
#include <vector>

#ifndef MYGL_POINT_H
#define MYGL_POINT_H

namespace mygl
{
  class Point: public Drawable //A Point is a Drawable
  {
    public:
      Point(VAO::model& vao, const glm::mat4& model, const glm::vec3& point, const glm::vec4& color, const float radius);
      virtual ~Point();

      virtual void DoDraw(mygl::ShaderProg& shader);

    private:
      const GLuint fNVertices; //Number of vertices in this path

      const float fRadius;
  };
}

#endif //MYGL_POINT_H
