//File: Noop.cpp
//Brief: Do nothing as advertised.  
//Author: Andrew Olivier aolivier@ur.rochester.edu

//model includes
#include "gl/model/Noop.h"

namespace mygl
{
  Noop::Noop(VAO::model& /*vao*/): Drawable(glm::mat4())
  {
  }

  void Noop::DoDraw(ShaderProg& /*shader*/)
  {
  }

  Noop::~Noop()
  {
  }
}
