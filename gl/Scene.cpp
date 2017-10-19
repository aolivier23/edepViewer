//File: Scene.cpp
//Brief: A Scene collects the resources to draw a group of related objects using opengl.   
//Author: Andrew Olivier aolivier@ur.rochester.edu

//view includes
#include "gl/Scene.h"
#include "gl/model/Drawable.h"

//c++ includes
#include <sstream>

namespace mygl
{
  //Warning: Make sure that Gtk::GLArea::make_current() is called just before this function so that 
  //         the proper GL context is bound for shader allocation.
  Scene::Scene(const std::string& name, const std::string& fragSrc, const std::string& vertSrc): fName(name), fShader(fragSrc, vertSrc), 
                                                                                                 fDrawables()
  {
  }

  Scene::~Scene() {}

  //Warning: Make sure that Gtk::GLArea::make_current() is called just before this function so that 
  //         the proper GL context is bound for resource allocation.
  void Scene::AddDrawable(std::unique_ptr<Drawable>&& drawable, const VisID& id)
  {
    const auto exists = fDrawables.find(id);
    if(exists != fDrawables.end()) 
    {
      std::stringstream str;
      for(auto& pair: fDrawables) str << pair.first << "\n";
      throw util::GenException("Duplicate ID") << "In view::Scene::AddDrawable(), tried to add a new "
                                               << "drawable with ID " << id << " in scene " << fName 
                                               << ", but an object already exists with this ID!\n"
                                               << "IDs so far are:\n" << str.str() << "\n";
    }
    else
    {
      fDrawables.emplace(id, std::move(drawable));
    }
  }

  void Scene::RemoveDrawable(const VisID& id)
  {
    auto exists = fDrawables.find(id);
    if(exists == fDrawables.end()) throw util::GenException("ID Does Not Exist") << "In view::Scene::RemoveDrawable(), tried to remove "
                                                                                 << "a drawable with ID " << id << " in scene " << fName 
                                                                                 << ", but there is no such object.\n";
    else
    {
      fDrawables.erase(exists);
    }
  }

  //Warning: Make sure that Gtk::GLArea::make_current() is called just before this function so that 
  //         the proper GL context is bound for rendering.
  void Scene::Render(const glm::mat4& view, const glm::mat4& persp) 
  {
    //Note that these uniform names assume that like-named uniforms are 
    //handled by the shader programs used to form fShader
    fShader.SetUniform("view", view);
    fShader.SetUniform("projection", persp);
    fShader.SetUniform("model", glm::mat4()); //glm::scale(glm::mat4(), glm::vec3(0.5f, 0.5f, 0.5f))); //TODO: allow Drawables to set their own matrices
    for(auto& pair: fDrawables) pair.second->Draw(fShader); //This is different from my old DRAWER contract because 
                                                            //Drawables are now responsible for binding their own 
                                                            //model matrices.
  }
}
