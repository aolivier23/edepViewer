//File: GLScene.h
//Brief: A GLScene manages a collection of Drawables.  It is a view component, 
//       so it does not do anything.  It provides an interface that an event display 
//       controller component (currently considering a GLViewer class) can use to 
//       transfer information to the visible view.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//view includes
#include "view/gl/GenException.h"

//gtkmm includes
#include <gtkmm.h>

//c++ includes
#include <map>
#include <limits>
#include <memory>
#include <iostream>

#ifndef VIEW_GLSCENE_H
#define VIEW_GLSCENE_H

namespace view
{
  class ShaderProg;
  class Drawable;

  //TODO: Putting this here for now, but it should probably move to somewhere common to all components
  //A VisID is an identified for a Drawable that can be converted to a color in the form of a glm::vec3 
  //for the picking interface I have in mind.
  struct VisID 
  {
    unsigned char fR;
    unsigned char fG;
    unsigned char fB;

    operator (glm::vec4)() const
    {
      return glm::vec4(fR, fG, fB, 0.0f);
    }

    VisID& operator ++() //prefix
    { 
      //Handle overflows
      const auto max = std::numeric_limits<unsigned char>::max();
      if(fB == max) fB = 0;
      {
        if(fG == max) fG = 0;
        {
          if(fR == max) throw util::GenException("Max VisID") << "Reached maximum VisID!  Identifiers for Drawables in "
                                                              << "GLScenes are no longer unique.  This will interferer with "
                                                              << "object selection.\n";
          else ++fR;
        }
        else ++fG;
      }
      else ++fB;
      return *this;
    }

    VisID operator ++(int) //postfix
    {
      VisID result = *this;
      ++(*this);
      return result;
    }
  };

  //Printing interface for VisID
  std::ostream& operator <<(std::ostream& os, const VisID& id)
  { 
    os << "(" << id.fR << ", " << id.fG << ", " << id.fB << ")";
    return os;
  }

  class GLScene
  {
    public:
      GLScene(const std::string& name, const std::string& fragSrc, const std::string& vertSrc)
      virtual ~GLScene() = default;

      //Warning: The following function may not do anything if Gtk::GLArea::make_current() is 
      //         not called just before.  
      virtual void AddDrawable(std::unqiue_ptr<Drawable>&& drawable, const VisID& id);
      virtual void RemoveDrawable(const VisID& id);

      //Draws all of the objects in fDrawables using fShader, view matrix, and persp(ective) matrix
      virtual void Render(const glm::mat4& view, const glm::mat4& persp);

      const std::string fName; //The name of this scene.  Might be useful for list tree displays

    protected:
      std::map<VisID, std::unique_ptr<Drawable>> fDrawables; //The opengl drawing instructions for this scene
      ShaderProg fShader; //The opengl shader program with which the objects in fDrawables will be rendered
  };
}

#endif //VIEW_GLVIEWER_H
