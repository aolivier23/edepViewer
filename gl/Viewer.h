//File: Viewer.h
//Brief: Creates an area for opengl drawing in a Gtk Widget.  Expects to be given constructor information for Drawables by 
//       external code, then draws those Drawables.  User can remove Drawables by VisID.  Drawables are managed by GLScenes.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//c++ includes
#include <memory> //for std::unique_ptr
#include <giomm/resource.h> //for Glib::RefPointer?
#include <type_traits> //for std::true_type and std::false_type

//local includes
#include "gl/camera/Camera.h"
#include "gl/Scene.h"
#include "gl/model/GenException.h"

//gtk includes
#include <gtkmm.h>

//c++ includes
#include <iostream>
#include <memory>

#ifndef MYGL_VIEWER_H
#define MYGL_VIEWER_H

namespace mygl
{
  struct VisID;
  
  class Viewer: public Gtk::Box //Instead of deriving from Gtk::GLArea, derive from Gtk::Box.  This suggestion to do this 
                                      //that I used is at https://github.com/mschwan/glarea-animation/blob/master/glarea.cc
  {
    protected:
      //opengl-related data members
      std::map<std::string, mygl::Scene> fSceneMap; //Map from Scene name to Scene object

      //Gtk::Widgets
      //TODO: Camera mode widget
      Gtk::GLArea fArea; //The GLArea that will be used for drawing

    public:
  
      Viewer(std::shared_ptr<Camera> cam, const float xPerPixel = 1, const float yPerPixel = 1, const float zPerPiexel = 1); //TODO: Camera mode GUI
      virtual ~Viewer();

      template <class T, class ...ARGS>
      void AddDrawable(const std::string& scene, const VisID& id, ARGS... args) //Add a drawable to a scene
      {
        auto scenePair = fSceneMap.find(scene);
        if(scenePair == fSceneMap.end())
        {
          std::stringstream scenes;
          for(const auto& scenePair: fSceneMap) scenes << scenePair.first << "\n";
          throw util::GenException("No Such Scene") << "In mygl::Viewer::AddDrawable(), could not add Drawable because scene " << scene << " does not exist.  "
                                                    << "The current list of scenes is:\n" << scenes.str() << "\n";
          return;
        }

        fArea.make_current(); //Make sure the resources allocated by the new Drawable go to the right Gdk::GLContext!
        fArea.throw_if_error();
        scenePair->second.AddDrawable(std::move(std::unique_ptr<Drawable>(new T(args...))), id);
      }

      //User access to Scenes
      void MakeScene(const std::string& name, const std::string& fragSrc = "/home/aolivier/app/evd/src/gl/shaders/userColor.frag", 
                     const std::string& vertSrc = "/home/aolivier/app/evd/src/gl/shaders/camera.vert");

      std::map<std::string, Scene>& GetScenes() { return fSceneMap; }

    protected:
      //TODO: Allow the user to set the camera to use in the future
      std::shared_ptr<Camera> fCamera; //Camera used by this GLArea
  
      virtual void realize();
      virtual void unrealize();
      virtual bool on_motion_notify_event(GdkEventMotion* /*evt*/);

      virtual bool render(const Glib::RefPtr<Gdk::GLContext>& /*context*/);

    private:
      float fXPerPixel; //x units per pixel
      float fYPerPixel; //y units per pixel
      float fZPerPixel; //z units per pixel
  };
}
#endif //End ifndef MYGL_VIEWER_H
