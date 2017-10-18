//File: GLCameraArea.h
//Brief: Creates an area with camera controls for drawing using opengl in a gtkmm Window.  I can add standard useful buttons here, 
//       and all of my early OpenGL practice will get those buttons.  This version using a camera object that satisfies a contract
//       rather than a dynamic interface.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//c++ includes
#include <memory> //for std::unique_ptr
#include <giomm/resource.h> //for Glib::RefPointer?
#include <type_traits> //for std::true_type and std::false_type

//Note: GLCameraArea is a class template, so c++ gets very upset if I separate declaration from implementation...
//local includes
#include "Camera.h"

//gtk includes
#include <gtkmm.h>

//c++ includes
#include <iostream>

//GLEW includes
#define GLEW_STATIC
#include "GL/glew.h"

#ifndef MYGL_GLCAMERAAERA_H
#define MYGL_GLCAMERAAREA_H

//TODO: I really want this to be a class derived from Gtk::GLArea that I add as a widget in a top level window.
namespace mygl
{
  template <class DRAWER> 
  //Template parameter: DRAWER: User-provided class that performs opengl drawing in the render() method.
  //                            DRAWER must implement: 
  //                            a.) DRAWER(): Initialize opengl resources.  
  //                            b.) Draw(const glm::mat4& view, const glm::mat4& persp): Do openGL drawing.  Called during render.  
  //                                GL_Clear() and GL_Clear_Color() calls are all that is provided, so user needs to call 
  //                                GL_DrawElements() call(s).  The camera's view and perspective matrices are passed to the 
  //                                DRAWER::Draw() function.   
  //                            c.) ~DRAWER(): Destroy opengl resources.  Called during unrealize().
  
  class GLCameraArea: public Gtk::Box //Instead of deriving from Gtk::GLArea, derive from Gtk::Box.  This suggestion to do this 
                                      //that I used is at https://github.com/mschwan/glarea-animation/blob/master/glarea.cc
  {
    public:
      //opengl-related data members
      std::unique_ptr<DRAWER> fDrawer; //Using unique_ptr so I can create and destroy DRAWER in realize()/unrealize().  
                                       //This way, DRAWER (should!) create and destroy its opengl resources like a 
                                       //"regular c++ class" (i.e. what LArSoft taught me until I learn better)

    protected:
      //The following are not constant because window resizing might become an issue later
      GLuint fWidth; //Window width
      GLuint fHeight; //Window height
      Gtk::GLArea fArea; //The GLArea that will be used for drawing

    public:
  
      GLCameraArea(const GLuint width, const GLuint height, std::shared_ptr<Camera> cam): 
                   Gtk::Box(Gtk::ORIENTATION_VERTICAL), fDrawer(nullptr), fWidth(width), fHeight(height), fArea(), fCamera(cam)
      {
        //Setup GLArea
        add(fArea);        

        //TODO: Key press/release is still not working in GLArea
        add_events(Gdk::SCROLL_MASK | Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK
                         | Gdk::KEY_PRESS_MASK | Gdk::KEY_RELEASE_MASK | Gdk::POINTER_MOTION_MASK);
        fArea.add_events(Gdk::SCROLL_MASK | Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK 
                         | Gdk::KEY_PRESS_MASK | Gdk::KEY_RELEASE_MASK | Gdk::POINTER_MOTION_MASK);
    
        fArea.set_required_version(3, 3);

        //Looks like window layout hints (ROOT's TGLayoutHints?)
        //TODO: Should the Window including this object call these?
        fArea.set_hexpand(true);
        fArea.set_vexpand(true);
        fArea.set_auto_render(true);
    
        //GLArea signals
        //TODO: Confirm that these work.  I am now overriding these methods directly for Gtk::GLArea
        fArea.signal_realize().connect(sigc::mem_fun(*this, &GLCameraArea::realize), false);
        fArea.signal_unrealize().connect(sigc::mem_fun(*this, &GLCameraArea::unrealize), false);
        fArea.signal_render().connect(sigc::mem_fun(*this, &GLCameraArea::render), false);
        fArea.signal_resize().connect(sigc::mem_fun(*this, &GLCameraArea::resize), false);

        //Configure opengl
        fArea.set_has_depth_buffer(true);

        fCamera->ConnectSignals(fArea);
        show_all();
      }

      virtual ~GLCameraArea() override {} //Do nothing for now

      void make_current()
      {
        fArea.make_current();
      }
  
    protected:
      //TODO: Allow the user to set the camera to use in the future
      std::shared_ptr<Camera> fCamera; //Camera used by this GLArea
  
      virtual void realize()
      {
        fArea.make_current();
        try
        {
          fArea.throw_if_error();
          glewExperimental = GL_TRUE;
          if(glewInit() != GLEW_OK)
          {
            std::cerr << "Failed to initialize GLEW\n";
          }

          glViewport(0, 0, fWidth, fHeight);

          //enable depth testing
          glEnable(GL_DEPTH_TEST);

          fDrawer.reset(new DRAWER());
          //DRAWER constructor takes the place of example's init_buffers and init_shaders
        }
        catch(const Gdk::GLError& err)
        {
          std::cerr << "Caught exception when instantiating DRAWER class in GLCameraArea::realize():\n"
                    << err.domain() << "-" << err.code() << "-" << err.what() << "\n";
          //TODO: Should I rethrow?
        }
      }

      virtual void unrealize()
      {
        fArea.make_current();
        try
        {
          fArea.throw_if_error();
          fDrawer.reset(nullptr); //DRAWER::~DRAWER() should be called here
        }
        catch(const Gdk::GLError& err)
        {
          std::cerr << "Caught exception when instantiating DRAWER class in GLCameraArea::unrealize():\n"
                    << err.domain() << "-" << err.code() << "-" << err.what() << "\n";
          //TODO: Should I rethrow?
        }
      }

      virtual bool render(const Glib::RefPtr<Gdk::GLContext>& /*context*/) 
      {
        try
        {
          fArea.throw_if_error();

          glClearColor(0.2f, 0.3f, 0.3f, 1.0f); //TODO: Allow user to set background color
          glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

          fDrawer->Draw(fCamera->GetView(), fCamera->GetPerspective(fWidth, fHeight));
          glFlush();

          //Force continuous rendering.  
          fArea.queue_render(); 

          return false; 
        }
        catch(const Gdk::GLError& err)
        {
          std::cerr << "Caught exception when instatiating DRAWER class in GLCameraArea::render():\n"
                    << err.domain() << "-" << err.code() << "-" << err.what() << "\n";
          return false;
        }
      }
 
      virtual void resize(int width, int height) 
      {
        std::cout << "Got call to mygl::GLCameraArea::resize() with width=" << width << " and height=" << height << "\n";
        fWidth = width;
        fHeight = height;
        glViewport(0, 0, width, height);
        fArea.queue_render();
      }

  };
}
#endif //End ifndef MYGL_GLCAMERAAREA_H

