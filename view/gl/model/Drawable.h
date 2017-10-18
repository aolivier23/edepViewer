//File: Drawable.h
//Brief: Abstract base class for any object can be drawn.  Will be collected by a dedicated gtkmm drawing widget.
//       Technical note to self: Making everything derive from Drawable lets me keep a container of object to draw.
//                               A compile-time solution might get ugly.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef DRAW_DRAWABLE_H
#define DRAW_DRAWABLE_H

//c++ includes that I couldn't seem to get away without
#include <string>

namespace view
{
  class ShaderProg;
}

namespace view
{
  class Drawable
  {
    public:
      Drawable();
      virtual ~Drawable() = default;

      //Interface to gtkmm widget.  All Drawables must implement this.
      virtual void Draw(mygl::ShaderProg& shader) = 0;
      //Add other interfaces here.  My current plan is that anything the 
      //drawing manager knows about should appear here.

    private: 
      //Add any other properties the drawing manager needs to know about here.  
      //TODO: How will different Drawables know to request different shader programs? 
      //      Thinking about some kind of template system on types of Uniforms and 
      //      VAO attribute, but that got very ugly last time...
  };
}
#endif //DRAW_DRAWABLE_H
