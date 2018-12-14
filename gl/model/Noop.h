//File: Noop.h
//Brief: A Noop is a Drawable that doesn't draw anything.  Creating a Noop in ctrl::SceneModel<>::view::emplace<>() lets 
//       users create "organization" nodes.  
//Author: Andrew Olivier aolivier@ur.rochester.edu

//model includes
#include "gl/model/Drawable.h"
#include "gl/objects/ShaderProg.h"
#include "gl/objects/VAO.h"

//c++ includes
#include <vector>

#ifndef MYGL_NOOP_H
#define MYGL_NOOP_H

namespace mygl
{
  class Noop: public Drawable //A Noop is a Drawable
  {
    public:
      Noop(VAO::model& /*vao*/);
      virtual ~Noop();

      virtual void DoDraw(mygl::ShaderProg& /*shader*/);
  };
}

#endif //MYGL_NOOP_H
