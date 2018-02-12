//File: PuglArea.h
//Brief: Replacement for Gtk::GLArea that actually works on OSX and doesn't just throw an exception.  
//       Pugl is a library that creates an OpenGL context on Window, OSX, and Linux (X11?) and provides 
//       support for embedding in a native window.  This class should put a Pugl window into a Gtk::DrawingArea 
//       with platform-specific code so that no one else needs to know the details.  I also want to API to look 
//       as much like Gtk::GLArea as possible.  
//Author: Andrew Olivier aolivier@ur.rochester.edu

//Define which APIs Pugl should support
#define PUGL_HAVE_GL 1
#define PUGL_HAVE_CAIRO 0

//Pugl includes
//#include "pugl/gl.h" //TODO: Do I need this here?
#include "pugl/pugl.hpp"

//gtkmm includes
#include <gtkmm.h>

#ifndef MYGL_PUGLAREA_H
#define MYGL_PUGLAREA_H

namespace mygl
{
  //Gtk-style Widget to encapsulate communication between Gtk and pugl.  Forwards pugl events into the Gtk main loop and 
  //ensures correct parentage in another Gtk::Widget.  
  class PuglArea: public pugl::View, public Gtk::DrawingArea
  {
    public:
      PuglArea(); //Initialize some settings
      virtual ~PuglArea() = default;

      //My own API to mimic the interface of Gtk::GLArea as much as possible
      void make_current(); //Make the opengl context of this area current
      void queue_render(); //Request that signal_render be called as soon as feasible

      void do_render(); //Do rendering to opengl context

      virtual void onEvent(const PuglEvent* evt) override; //Forward events into the Gtk main loop
      //Sounds like I want to call gtk_main_do_event() in onEvent(): https://developer.gnome.org/gtk3/stable/gtk3-General.html

      //Call opengl drawing functions by reacting to this signal.  
      //Set up signal for doing opengl stuff before rendering to the pugl::View.  
      typedef sigc::signal<void> SignalRender;
      SignalRender signal_render();

      //Custom exception class that I ripped off from my util library
      class exception
      {
        public:
          exception(const std::string& category) noexcept
          {
            fExplanation << category << ": ";
          }
          exception(const exception& other) noexcept: fExplanation(other.fExplanation.str())
          {
          }
  
          exception& operator =(const exception& rhs) noexcept
          {
            fExplanation.clear();
            fExplanation << rhs.what();
            return *this;
          }
    
          virtual ~exception() noexcept = default;
    
          virtual std::string what() const
          {
            return fExplanation.str();
          }
    
          template <class T>
          exception& operator << (const T& toPrint)
          {
            fExplanation << toPrint;
            return *this;
          }
    
        protected:
          std::stringstream fExplanation; //Explaination of why this exception was thrown. 
      };

      //Handle some gtk signals that let me do things with pugl
      virtual void on_realize() override;
      /*virtual void on_show() override;
      virtual void on_hide() override;
      virtual void on_size_allocate(Gdk::Rectangle& rect) override;*/

      //virtual bool on_draw(const ::Cairo::RefPtr< ::Cairo::Context >& ctx) override;

    protected:
      //Signal that it is time to call opengl drawing functions.
      SignalRender fRender;
      typedef sigc::signal<void> SignalDoRender;
      SignalDoRender fDoRender;
  };
}

#endif //MYGL_PUGLAREA_H 
