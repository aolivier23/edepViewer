//File: PuglArea.cpp
//Brief: Replacement for Gtk::GLArea that actually works on OSX and doesn't just throw an exception.  
//       Pugl is a library that creates an OpenGL context on Window, OSX, and Linux (X11?) and provides 
//       support for embedding in a native window.  This class should put a Pugl window into a Gtk::DrawingArea 
//       with platform-specific code so that no one else needs to know the details.  I also want to API to look 
//       as much like Gtk::GLArea as possible.  
//Author: Andrew Olivier aolivier@ur.rochester.edu

//local includes
#include "gl/PuglArea.h"

//Includes for gdk's interface to various windowing systems.  Learned to do this from https://gstreamer.freedesktop.org/documentation/tutorials/basic/toolkit-integration.html
#include <gdk/gdk.h>
#if defined (GDK_WINDOWING_X11)
#include <gdk/gdkx.h>
#elif defined (GDK_WINDOWING_WIN32)
#include <gdk/gdkwin32.h>
#elif defined (GDK_WINDOWING_QUARTZ)
#include <gdk/gdkquartz.h>
#endif

//include opengl enums from pugl
#include "pugl/gl.h"
//#include "glad/include/glad/glad.h"

//c++ includes
#include <iostream> //TODO: remove me

/*namespace
{
  //Error handling callback for opengl
  void glErr(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, GLchar* message, const void* userParam)
  {
    throw mygl::PuglArea::exception("OpenGL Error") << "Got opengGL error: " << message
                                                    << "from source: " << source << "\n"
                                                    << "of type: " << type << "\n"
                                                    << "with id: " << id << "\n"
                                                    << "of severity " << severity << "\n";
  }
}*/ //Only works with opengl >= 4.3 :(

namespace mygl
{
  //Initialize PuglArea.  Can't set window parent at this point because Gtk::DrawingArea hasn't been realized yet.  
  PuglArea::PuglArea(): pugl::View(nullptr, nullptr), Gtk::DrawingArea()
  {
    //Forward the drawing area's properties to pugl
    initResizable(true); //TODO: Are all Gtk::Widgets resizable?

    //Make sure queue_render() sets up an eventual call to do_render().
    fDoRender.connect(sigc::mem_fun(*this, &PuglArea::do_render));

    set_has_window(true);

    set_size_request(100, 100);

    //show_all();
  }

  //Make the opengl context behind this pugl context the current context to which changes are posted
  void PuglArea::make_current()
  {
    puglEnterContext(cobj());
  }

  PuglArea::SignalRender PuglArea::signal_render()
  {
    return fRender;
  }

  //Request that signal_render be called as soon as feasible
  void PuglArea::queue_render()
  {
    fDoRender.emit();
  }

  //TODO: This is not getting called.  
  //Realize a pugl::View using a platform-specific parent window API
  void PuglArea::on_realize()
  {
    Gtk::DrawingArea::on_realize();
    auto window = get_window()->gobj();
 
    if(!gdk_window_ensure_native(window))
    {
      throw exception("PuglArea::on_realize()") << "Gdk could not get a native window.\n";
    }

    #if defined (GDK_WINDOWING_X11) //If using an X11 window system
      initWindowParent(gdk_x11_window_get_xid(window));
    #elif defined (GDK_WINDOWING_QUARTZ) //If using a Quartz window system
      initWindowParent(gdk_quartz_window_get_nsview(window));
    #elif defined (GDK_WINDOWING_WIN32) //If using Windows 32 bit.  I'll admit that I don't plan to test this one soon.
      initWindowParent((guintptr)GDK_WINDOW_HWND(window));
    #else
      throw exception("mygl::PuglArea::on_realize()") << "This native window system is not supported.\n";
    #endif

    initContextType(PuglContextType::PUGL_GL);
    createWindow("PuglArea"); //TODO: Get window name from gtk somehow?
    std::cout << "Created pugl window in PuglArea::on_realize().  Am I ready to make its' context current?\n";
    make_current();
    std::cout << "Made opengl context current for version check.\n";

    //Check the opengl version
    /*GLint* major, *minor;
    glGetIntegerv(GL_MAJOR_VERSION, major);
    glGetIntegerv(GL_MINOR_VERSION, minor);
    std::cout << "Pugl created an opengl " << major[0] << "." << minor[0] << " context.\n";

    //Check for context creation errors?
    GLenum err;
    std::string errors;
    while((err = glGetError()) != GL_NO_ERROR)
    {
      if(err == GL_INVALID_ENUM) errors += "GLInvalidEnum\n";
      else if(err == GL_INVALID_VALUE) errors += "GLInvalidValue\n";
      else if(err == GL_INVALID_OPERATION) errors += "GLInvalidOperation\n";
      //else if(err == GL_STACK_OVERFLOW) errors += "GLStackOverflow\n";
      //else if(err == GL_STACK_UNDERFLOW) errors += "GLStackUnderflow\n";
      else if(err == GL_OUT_OF_MEMORY) errors += "GL_OUT_OF_MEMORY\n";
      else if(err == GL_INVALID_FRAMEBUFFER_OPERATION) errors += "GL_INVALID_FRAMEBUFFER_OPERATION\n";
    }
    if(!errors.empty()) throw exception("Opengl errors in on_realize()") << "Got the following opengl errors in render: " << errors;*/

    //Set up basic error logging
    //make_current();

    //Needs opengl >= 4.3
    //glEnable(GL_DEBUG_OUTPUT);
    //glDebugMessageCallback(::glErr, nullptr);
  }

  /*void PuglArea::on_show()
  {
    //showWindow();
    Gtk::DrawingArea::show();
  }*/

  /*void PuglArea::on_hide()
  {
    //hideWindow();
    Gtk::DrawingArea::hide();
  }*/

  /*void PuglArea::on_size_allocate(Gdk::Rectangle& rect)
  {
    Gtk::DrawingArea::on_size_allocate(rect);
    initWindowSize(rect.get_x(), rect.get_y());
  }*/

  void PuglArea::do_render()
  {
    //TODO: I have had to enclose rendering calls with make_current() in the past.  Maybe I could remove that requirement by drawing 
    //      through do_render()?  Perhaps a more important question is why GLArea didn't do this in the first place.  Something is 
    //      very wrong.  
    //puglEnterContext(cobj()); //Make context current
    fRender.emit(); //Give other widgets the chance to react to this context change
    /*GLenum err;
    std::string errors;
    while((err = glGetError()) != GL_NO_ERROR)
    {
      if(err == GL_INVALID_ENUM) errors += "GLInvalidEnum\n";
      else if(err == GL_INVALID_VALUE) errors += "GLInvalidValue\n";
      else if(err == GL_INVALID_OPERATION) errors += "GLInvalidOperation\n";
      //else if(err == GL_STACK_OVERFLOW) errors += "GLStackOverflow\n";
      //else if(err == GL_STACK_UNDERFLOW) errors += "GLStackUnderflow\n";
      else if(err == GL_OUT_OF_MEMORY) errors += "GL_OUT_OF_MEMORY\n";
      else if(err == GL_INVALID_FRAMEBUFFER_OPERATION) errors += "GL_INVALID_FRAMEBUFFER_OPERATION\n";
    }
    if(!errors.empty()) throw exception("Opengl errors in do_render()") << "Got the following opengl errors in render: " << errors;*/
    puglLeaveContext(cobj(), true); //Apply changes to context
  }

  /*bool PuglArea::on_draw(const ::Cairo::RefPtr< ::Cairo::Context >& ctx)
  {
    do_render();
    return false;
  }*/

  //Forward events into the Gtk main loop.  Leaving this empty to try to forward events to the gtk widget instead.
  //Sounds like I want to call gtk_main_do_event() in onEvent(): https://developer.gnome.org/gtk3/stable/gtk3-General.html
  void PuglArea::onEvent(const PuglEvent* evt)
  { 
    //Empty implementation
    //TODO: Will this ever be called, or will the Gtk::DrawingArea base class take care of events?  I'd rather avoid running the 
    //      pugl event loop if I can. 
    //TODO: Convert a pugl::Event to a GdkEvent and forward it to the gtk main loop
  }
}
