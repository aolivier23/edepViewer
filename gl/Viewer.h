//File: Viewer.h
//Brief: Creates an area for opengl drawing in a Gtk Widget.  Expects to be given constructor information for Drawables by 
//       external code, then draws those Drawables.  User can remove Drawables by VisID.  Drawables are managed by Scenes.
//       Viewer also provides a GUI area for configuring opengl drawing and Cameras.  Viewer manages the current Camera and 
//       the list of Cameras configured.
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
  
  class Viewer: public Gtk::Paned //Instead of deriving from Gtk::GLArea, derive from Gtk::Box.  This suggestion to do this 
                                      //that I used is at https://github.com/mschwan/glarea-animation/blob/master/glarea.cc
  {
    protected:
      //opengl-related data members
      std::map<std::string, mygl::Scene> fSceneMap; //Map from Scene name to Scene object

      //Gtk::Widgets
      //TODO: Camera mode widget
      Gtk::GLArea fArea; //The GLArea that will be used for drawing

    public:
  
      Viewer(std::unique_ptr<Camera>&& cam, const Gdk::RGBA& background, const float xPerPixel = 1, const float yPerPixel = 1, const float zPerPiexel = 1); 
      virtual ~Viewer();

      //TODO: At first, this function seems superfluous if I give the user access to the map of scenes.  However, the Gtk::GLArea::make_current() call 
      //      here is essential to properly managing the opengl resources in a Drawable.  So, maybe I should remove user access to the scenes a Viewer 
      //      owns?  This will require adding entry points for basically all of the Scene public functions as well as each Scene's TreeView.
      template <class T, class ...ARGS>
      Gtk::TreeModel::Row AddDrawable(const std::string& scene, const VisID& id, const Gtk::TreeModel::Row& parent, 
                                      const bool active = true, ARGS... args) //Add a drawable to a scene
      {
        auto scenePair = fSceneMap.find(scene);
        if(scenePair == fSceneMap.end())
        {
          std::stringstream scenes;
          for(const auto& scenePair: fSceneMap) scenes << scenePair.first << "\n";
          throw util::GenException("No Such Scene") << "In mygl::Viewer::AddDrawable(), could not add Drawable because scene " << scene << " does not exist.  "
                                                    << "The current list of scenes is:\n" << scenes.str() << "\n";
        }

        fArea.make_current(); //Make sure the resources allocated by the new Drawable go to the right Gdk::GLContext!
        fArea.throw_if_error();
        auto row = scenePair->second.AddDrawable(std::move(std::unique_ptr<Drawable>(new T(args...))), id, parent, active);
        return row;
      }

      //User access to Scenes
      Gtk::TreeView& MakeScene(const std::string& name, mygl::ColRecord& cols, const std::string& fragSrc = "/home/aolivier/app/evd/src/gl/shaders/userColor.frag", 
                     const std::string& vertSrc = "/home/aolivier/app/evd/src/gl/shaders/camera.vert");
      Gtk::TreeView& MakeScene(const std::string& name, mygl::ColRecord& cols, const std::string& fragSrc, const std::string& vertSrc,
                               const std::string& geomSrc);

      //Makes sure openGL context is current before destroying Drawables.  
      void RemoveAll(const std::string& sceneName);

      //TODO: Consider removing this function.  It violates the concept of the Viewer making sure that its' GLArea is current when its' Scenes are modified.
      std::map<std::string, Scene>& GetScenes() { return fSceneMap; }

      void AddCamera(const std::string& name, std::unique_ptr<Camera>&& camera);

      bool on_click(GdkEventButton* evt); //Handle user selection of drawn objects

      //Int parameters useful if I want to draw a tooltip
      typedef sigc::signal<void, const mygl::VisID/*, const int, const int*/> SignalSelection;
      SignalSelection signal_selection(); //User access to this Viewer's signal that it selected an object

      void on_selection(const mygl::VisID id/*, const int, const int*/);
 
    protected:
      //Viewer parameters the user can customize
      //TODO: Allow the user to set the camera to use in the future
      Gdk::RGBA fBackgroundColor; //The background color for the display
  
      virtual void area_realize();
      virtual void unrealize();
      virtual bool my_motion_notify_event(GdkEventMotion* /*evt*/);

      virtual bool render(const Glib::RefPtr<Gdk::GLContext>& /*context*/);

      //Other signals to react to 
      virtual void set_background();

      //GUI elements
      Gtk::Notebook fNotebook;
      std::vector<std::pair<Gtk::Box, Gtk::ScrolledWindow>> fScrolls; //One for each Scene

      //Viewer control GUI
      Gtk::Box fControl;
      Gtk::Label fBackColorLabel;
      Gtk::ColorButton fBackgroundButton;
      //TODO: Line width control

      //Camera selection GUI.  Camera controls provided by Camera and derived classes.
      Gtk::Stack fCameras;  //The list of cached Cameras
      Gtk::StackSwitcher fCameraSwitch; //Controller for fCameras.  Currently viewed Camera GUI is current Camera.   
      Camera* fCurrentCamera; //Observer pointer to fCameras' currently visible child to make code easier to understand

      void camera_change();

    private:
      float fXPerPixel; //x units per pixel
      float fYPerPixel; //y units per pixel
      float fZPerPixel; //z units per pixel

      void PrepareToAddScene(const std::string& name);
      Gtk::TreeView& ConfigureNewScene(const std::string& name, mygl::Scene& scene, mygl::ColRecord& cols);      

      SignalSelection fSignalSelection; 
  };
}
#endif //End ifndef MYGL_VIEWER_H
