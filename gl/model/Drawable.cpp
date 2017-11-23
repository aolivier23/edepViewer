//File: Drawable.cpp
//Brief: Implementation of the abstract base class for any object that my gtkmm drawing manager can draw.
//       Note: This is mainly here to make CMake happy.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//c++ includes
#include <string>

//local includes
#include "gl/model/Drawable.h"
#include "gl/model/ShaderProg.h"

namespace mygl
{
  Drawable::Drawable(const glm::mat4& model): fModel(model)
  {
  }

  Drawable::~Drawable() 
  {
  }

  void Drawable::Draw(mygl::ShaderProg& prog)
  {
    prog.Use(); //TODO: Rely on Scene to do this?
    prog.SetUniform("model", fModel);
    DoDraw(prog);
  }
}
