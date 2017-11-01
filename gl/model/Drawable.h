//File: Drawable.h
//Brief: Abstract base class for any object can be drawn.  Will be collected by a dedicated gtkmm drawing widget.
//       Technical note to self: Making everything derive from Drawable lets me keep a container of object to draw.
//                               A compile-time solution might get ugly.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef DRAW_DRAWABLE_H
#define DRAW_DRAWABLE_H

//glm includes
#include <glm/glm.hpp>

//c++ includes that I couldn't seem to get away without
#include <string>

namespace mygl
{
  class ShaderProg;
}

namespace mygl
{
  class Drawable
  {
    public:
      Drawable(const glm::mat4& model);
      virtual ~Drawable();

      //Public interface for gtkmm Widgets.  
      void Draw(mygl::ShaderProg& shader);
      //Add other interfaces here.  My current plan is that anything the 
      //drawing manager knows about should appear here.

    protected:
      const glm::mat4 fModel; //Describes the origin of the coordinate system in which to place this Drawable

      //All Drawables must implement this.
      virtual void DoDraw(mygl::ShaderProg& shader) = 0;

    private: 
      //Add any other properties the drawing manager needs to know about here.  
      //TODO: How will different Drawables know to request different shader programs? 
      //      Thinking about some kind of template system on types of Uniforms and 
      //      VAO attribute, but that got very ugly last time...
  };
}
#endif //DRAW_DRAWABLE_H
