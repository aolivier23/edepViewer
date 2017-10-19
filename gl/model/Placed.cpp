//File: Placed.cpp
//Brief: Placed is an attribute to add on to a Drawable as a mixin (like Placed<PolyMesh>).  
//       A Placed Drawable takes a model matrix and the Drawable's normal arguments.   The model 
//       matrix determines the origin of the drawing coordinate system.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//model includes
#include "gl/model/ShaderProg.h"

//glm includes
#include <glm/glm.hpp>

#ifndef MYGL_PLACED_CPP
#define MYGL_PLACED_CPP

namespace mygl
{
  template <class DRAWABLE>
  class Placed: public DRAWABLE
  {
    public:
      template <class ...ARGS>
      Placed(const glm::mat4& model, ARGS... args): DRAWABLE(args...), fModel(model)
      {
      }

      void Draw(ShaderProg& shader)
      {
        shader.SetUniform("model", fModel);
        DRAWABLE::Draw(shader);
      }

    private:
      const glm::mat4 fModel; //Describes the origin of the coordinate system in which to place this DRAWABLE
  };
}

#endif //MYGL_PLACED_CPP
