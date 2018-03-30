//File: Viewer.h
//Brief: Creates an area for opengl drawing in a Gtk Widget.  Expects to be given constructor information for Drawables by 
//       external code, then draws those Drawables.  User can remove Drawables by VisID.  Drawables are managed by Scenes.
//       Viewer also provides a GUI area for configuring opengl drawing and Cameras.  Viewer manages the current Camera and 
//       the list of Cameras configured.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//c++ includes
#include <memory> //for std::unique_ptr
#include <type_traits> //for std::true_type and std::false_type

//local includes
#include "gl/camera/Camera.h"
#include "gl/Scene.h"
#include "gl/model/GenException.h"

//glm includes
#include <glm/glm.hpp>

//c++ includes
#include <iostream>
#include <memory>

#ifndef MYGL_VIEWER_H
#define MYGL_VIEWER_H

namespace mygl
{
  struct VisID;
  
  class Viewer //Instead of deriving from Gtk::GLArea, derive from Gtk::Box.  This suggestion to do this 
               //that I used is at https://github.com/mschwan/glarea-animation/blob/master/glarea.cc
  {
    protected:
      //opengl-related data members
      std::map<std::string, mygl::Scene> fSceneMap; //Map from Scene name to Scene object
      std::map<std::string, mygl::Scene>::iterator fCurrentScene; //The Scene whose list tree is currently being displayed

    public:
  
      Viewer(std::unique_ptr<Camera>&& cam, const float xPerPixel = 1, const float yPerPixel = 1, const float zPerPiexel = 1); 
      virtual ~Viewer();

      //TODO: At first, this function seems superfluous if I give the user access to the map of scenes.  However, the Gtk::GLArea::make_current() call 
      //      here is essential to properly managing the opengl resources in a Drawable.  So, maybe I should remove user access to the scenes a Viewer 
      //      owns?  This will require adding entry points for basically all of the Scene public functions as well as each Scene's TreeView.
      template <class T, class ...ARGS>
      TreeModel::iterator AddDrawable(const std::string& scene, const VisID& id, const TreeModel::iterator parent, 
                                      const bool active, ARGS... args) //Add a drawable to a scene
      {
        auto scenePair = fSceneMap.find(scene);
        if(scenePair == fSceneMap.end())
        {
          std::stringstream scenes;
          for(const auto& scenePair: fSceneMap) scenes << scenePair.first << "\n";
          throw util::GenException("No Such Scene") << "In mygl::Viewer::AddDrawable(), could not add Drawable because scene " << scene << " does not exist.  "
                                                    << "The current list of scenes is:\n" << scenes.str() << "\n";
        }

        return scenePair->second.AddDrawable(std::move(std::unique_ptr<Drawable>(new T(args...))), id, parent, active);
      }

      //Use overloads so that this compiles with Clang as well as gcc.  See https://stackoverflow.com/questions/34494765/interaction-between-default-arguments-and-parameter-pack-gcc-and-clang-disagree

      template <class T, class ...ARGS>
      TreeModel::iterator AddDrawable(const std::string& scene, const VisID& id, const TreeModel::iterator parent,
                                      ARGS... args) //Add a drawable to a scene
      {
        return AddDrawable(scene, id, parent, true, args...);
      }

      //User access to Scenes
      void MakeScene(const std::string& name, std::shared_ptr<mygl::ColRecord> cols, const std::string& fragSrc = INSTALL_GLSL_DIR "/userColor.frag", 
                     const std::string& vertSrc = INSTALL_GLSL_DIR "/camera.vert");
      void MakeScene(const std::string& name, std::shared_ptr<mygl::ColRecord> cols, const std::string& fragSrc, const std::string& vertSrc,
                     const std::string& geomSrc);

      //Makes sure openGL context is current before destroying Drawables.  
      void RemoveAll(const std::string& sceneName);

      //TODO: Consider removing this function.  It violates the concept of the Viewer making sure that its' GLArea is current when its' Scenes are modified.
      std::map<std::string, Scene>& GetScenes() { return fSceneMap; }

      //Interface for interacting with the list of Cameras from plugins.  
      void AddCamera(const std::string& name, std::unique_ptr<Camera>&& camera); //Add a new Camera to the list of Cameras managed by this Viewer
      Camera& GetCamera(const std::string& name); //Get a Camera by name
      void RemoveCamera(const std::string& name); //Remove a Camera by name.  Resets to the Default Camera if the current Camera is removed, and 
                                                  //refuses to remove the last Camera.

      bool on_click(const int button, const float x, const float y, const int width, const int height); //Handle user selection of drawn objects

      //Int parameters useful if I want to draw a tooltip
      //typedef sigc::signal<void, const mygl::VisID/*, const int, const int*/> SignalSelection;
      //SignalSelection signal_selection(); //User access to this Viewer's signal that it selected an object

      void on_selection(const mygl::VisID id/*, const int, const int*/);

      void Render(const int width, const int height, const ImGuiIO& ioState); //Application/main window calls this in each frame
 
    protected:
      //Viewer parameters the user can customize
      glm::vec3 fBackgroundColor;
  
      virtual void area_realize();
      virtual void unrealize();
      //virtual bool my_motion_notify_event(GdkEventMotion* /*evt*/);

      virtual void render(const int width, const int height);

      //Other signals to react to 
      virtual void set_background();

      //Switched to a std::list here so that iterators are valid even after insertions and deletions
      std::map<std::string, std::unique_ptr<Camera>> fCameras;
      std::map<std::string, std::unique_ptr<Camera>>::iterator fCurrentCamera; //Observer pointer to fCameras' currently visible child to make code easier to understand

      void camera_change();

    private:
      float fXPerPixel; //x units per pixel
      float fYPerPixel; //y units per pixel
      float fZPerPixel; //z units per pixel

      void PrepareToAddScene(const std::string& name);
      void ConfigureNewScene(const std::string& name, mygl::Scene& scene, mygl::ColRecord& cols);      

      //TODO: Decide if I still want to use a signal for this task.  It seems to map pretty well onto 
      //      a strategy based around callbacks.  
      //SignalSelection fSignalSelection; 
  };
}
#endif //End ifndef MYGL_VIEWER_H
