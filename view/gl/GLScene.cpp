//File: GLScene.cpp
//Brief: A GLScene collects the resources to draw a group of related objects using opengl.   
//Author: Andrew Olivier aolivier@ur.rochester.edu

//view includes
#include "view/gl/GLScene.h"
#include "view/gl/model/Drawable.h"
#include "view/gl/model/ShaderProg.h"

namespace view
{
  //Warning: Make sure that Gtk::GLArea::make_current() is called just before this function so that 
  //         the proper GL context is bound for shader allocation.
  GLScene::GLScene(const std::string& name, const std::string& fragSrc, const std::string& vertSrc): fName(name), fShader(fracSrc, vertSrc)
  {
  }

  //Warning: Make sure that Gtk::GLArea::make_current() is called just before this function so that 
  //         the proper GL context is bound for resource allocation.
  void GLScene::AddDrawable(std::unique_ptr<Drawable>&& drawable, const VisID& id)
  {
    const auto exists = fDrawables.find(id);
    if(exists != fDrawables.end()) throw GenException("Duplicate ID") << "In view::GLScene::AddDrawable(), tried to add a new "
                                                                      << "drawable with ID " << id << " in scene " << fName 
                                                                      << ", but an object already exists with this ID!\n";
    else
    {
      fDrawables.emplace_back(id, drawable);
    }
  }

  void GLScene::RemoveDrawable(const VisID& id)
  {
    auto exists = fDrawables.find(id);
    if(exists == fDrawables.end()) throw GenException("ID Does Not Exist") << "In view::GLScene::RemoveDrawable(), tried to remove "
                                                                           << "a drawable with ID " << id << " in scene " << fName 
                                                                           << ", but there is no such object.\n";
    else
    {
      fDrawables.remove(exists);
    }
  }

  //Warning: Make sure that Gtk::GLArea::make_current() is called just before this function so that 
  //         the proper GL context is bound for rendering.
  void GLScene::Render(const glm::mat4& view, const glm::mat4& persp) 
  {
    //Note that these uniform names assume that like-named uniforms are 
    //handled by the shader programs used to form fShader
    fShader.SetUniform("view", view);
    fShader.SetUniform("projectionn", persp);
    for(auto pair: fDrawables) pair->second->Draw(fShader); //This is different from my old DRAWER contract because 
                                                            //Drawables are now responsible for binding their own 
                                                            //model matrices.
  }
}
