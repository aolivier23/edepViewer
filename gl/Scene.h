//File: Scene.h
//Brief: A Scene manages a collection of Drawables.  It is a view component, 
//       so it does not do anything.  It provides an interface that an event display 
//       controller component (currently considering a GLViewer class) can use to 
//       transfer information to the visible view.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//model includes
#include "gl/model/GenException.h"
#include "gl/model/ShaderProg.h"
#include "gl/VisID.h"

//glm includes
#include <glm/glm.hpp>

//gtkmm includes
#include <gtkmm.h>

//c++ includes
#include <map>
#include <limits>
#include <memory>
#include <iostream>

#ifndef VIEW_SCENE_H
#define VIEW_SCENE_H

namespace mygl
{
  class ShaderProg;
  class Drawable;

  class Scene
  {
    public:
      Scene(const std::string& name, const std::string& fragSrc, const std::string& vertSrc);
      virtual ~Scene();

      //Warning: The following function may not do anything if Gtk::GLArea::make_current() is 
      //         not called just before.  
      virtual void AddDrawable(std::unique_ptr<Drawable>&& drawable, const VisID& id);
      virtual void RemoveDrawable(const VisID& id);

      //Draws all of the objects in fDrawables using fShader, view matrix, and persp(ective) matrix
      virtual void Render(const glm::mat4& view, const glm::mat4& persp);

      const std::string fName; //The name of this scene.  Might be useful for list tree displays

    protected:
      std::map<VisID, std::unique_ptr<Drawable>> fDrawables; //The opengl drawing instructions for this scene
      ShaderProg fShader; //The opengl shader program with which the objects in fDrawables will be rendered
  };
}

#endif //VIEW_SCENE_H
