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
#include "gl/scene/SceneController.h"
#include "util/GenException.h"
#include "gl/scene/SceneConfig.cpp"

//glm includes
#include <glm/glm.hpp>

//c++ includes
#include <memory>
#include <map>

#ifndef MYGL_VIEWER_H
#define MYGL_VIEWER_H

namespace ctrl
{
  class ColumnModel;
  class SceneController;
}

namespace mygl
{
  struct VisID;
  
  class Viewer
  {
    protected:
      //opengl-related data members
      std::map<std::string, ctrl::SceneController> fSceneMap; //Map from Scene name to Scene object
      std::map<std::string, ctrl::SceneController>::iterator fCurrentScene; //The Scene whose list tree is currently being displayed

    public:
  
      Viewer(std::unique_ptr<Camera>&& cam, const float xPerPixel = 1, const float yPerPixel = 1, const float zPerPiexel = 1); 
      virtual ~Viewer();

      //Use overloads so that this compiles with Clang as well as gcc.  See https://stackoverflow.com/questions/34494765/interaction-between-default-arguments-and-parameter-pack-gcc-and-clang-disagree

      //User access to Scenes. 
      //TODO: This will probably become a function template when SceneController becomes a base class
      ctrl::SceneController& MakeScene(const std::string& name, std::shared_ptr<ctrl::ColumnModel> cols, 
                                       const std::string& fragSrc = INSTALL_GLSL_DIR "/userColor.frag", 
                                       const std::string& vertSrc = INSTALL_GLSL_DIR "/camera.vert", 
                                       std::unique_ptr<SceneConfig>&& config = std::unique_ptr<SceneConfig>(new SceneConfig()));
      ctrl::SceneController& MakeScene(const std::string& name, std::shared_ptr<ctrl::ColumnModel> cols, const std::string& fragSrc, 
                                       const std::string& vertSrc, const std::string& geomSrc, 
                                       std::unique_ptr<SceneConfig>&& config = std::unique_ptr<SceneConfig>(new SceneConfig()));

      //Interface for interacting with the list of Cameras from plugins.  
      void LoadCameras(std::map<std::string, std::unique_ptr<mygl::Camera>>&& cameraToName = std::map<std::string, std::unique_ptr<Camera>>());

      //User interaction 
      bool on_click(const int button, const float x, const float y, const int width, const int height); //Handle user selection of drawn objects

      void on_selection(const mygl::VisID id/*, const int, const int*/);

      //Application/main window calls this in each frame
      void Render(const int width, const int height, const ImGuiIO& ioState); 
 
    protected:
      //Viewer parameters the user can customize
      glm::vec3 fBackgroundColor;
  
      virtual void area_realize();
      virtual void unrealize();

      virtual void render(const int width, const int height);

      //Switched to a std::list here so that iterators are valid even after insertions and deletions
      std::map<std::string, std::unique_ptr<Camera>> fCameras;
      std::map<std::string, std::unique_ptr<Camera>>::iterator fCurrentCamera; //Observer pointer to fCameras' currently visible child to make code easier to understand
      std::unique_ptr<Camera> fDefaultCamera; //If no other Cameras are provided, use this one.  
      std::unique_ptr<Camera>& GetCurrentCamera(); 

      void camera_change();

    private:
      float fXPerPixel; //x units per pixel
      float fYPerPixel; //y units per pixel
      float fZPerPixel; //z units per pixel

      void PrepareToAddScene(const std::string& name);
      void ConfigureNewScene(const std::string& name, ctrl::SceneController& scene, ctrl::ColumnModel& cols);      
  };
}
#endif //End ifndef MYGL_VIEWER_H
