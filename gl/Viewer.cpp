//File: Viewer.h
//Brief: Creates an area for opengl drawing in a Gtk Widget.  Expects to be given constructor information for Drawables by 
//       external code, then draws those Drawables.  User can remove Drawables by VisID.  Drawables are managed by GLScenes.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//c++ includes
#include <memory> //for std::unique_ptr

//local includes
#include "gl/Viewer.h"
#include "gl/camera/Camera.h"

//imgui includes
#include "imgui.h"

//glm includes
#include <glm/gtc/type_ptr.hpp>

//c++ includes
#include <sstream>
#include <utility> //For std::piecewise_construct shenanigans
#include <tuple> //For std::forward_as_tuple

#include "glad/include/glad/glad.h"

namespace mygl
{
  Viewer::Viewer(std::unique_ptr<Camera>&& cam, const float xPerPixel, const float yPerPixel, const float zPerPixel):
                fSceneMap(), 
                fCurrentScene(fSceneMap.end()),
                fBackgroundColor(0., 0., 0.),
                fCameras(), 
                fXPerPixel(xPerPixel), fYPerPixel(yPerPixel), fZPerPixel(zPerPixel)
  {
    fDefaultCamera = std::move(cam);
    LoadCameras(); 

    //fSignalSelection.connect(sigc::mem_fun(*this, &Viewer::on_selection));

    //Configure opengl
    //TODO: Do this in GLFW-facing backend now?
    //fArea.set_has_depth_buffer(true);

    //fCameras.property_visible_child().signal_changed().connect(sigc::mem_fun(*this, &Viewer::camera_change));
    area_realize();
  }

  Viewer::~Viewer() {}

  //TODO: Rename this and/or render() so that the differences between them are more clear. 
  void Viewer::Render(const int width, const int height, const ImGuiIO& ioState)
  {
    ImGui::SetNextWindowPos(ImVec2(ioState.DisplaySize.x, 35.f), ImGuiCond_Always, ImVec2(1.0f, 0.f));
    ImGui::SetNextWindowSize(ImVec2(ioState.DisplaySize.x/4., ioState.DisplaySize.y-35.f), ImGuiCond_Appearing);
    ImGui::Begin("Viewer", nullptr, ImGuiWindowFlags_ResizeFromAnySide | ImGuiWindowFlags_AlwaysVerticalScrollbar); 
    ImGui::Columns(fSceneMap.size()+1);

    //Render selectable text for each Scene.  Basically, a poor-man's tab widget.
    bool selected = (fCurrentScene == fSceneMap.end());
    ImGui::Selectable("Viewer", &selected);
    ImGui::NextColumn();
    if(selected) fCurrentScene = fSceneMap.end();
    for(auto scene = fSceneMap.begin(); scene != fSceneMap.end(); ++scene) 
    {
      selected = (fCurrentScene == scene);
      ImGui::Selectable(scene->first.c_str(), &selected);
      if(selected) 
      {
        fCurrentScene = scene;
      }
      ImGui::NextColumn();
    }
    ImGui::Columns(1);

    ImGui::Separator();
    if(fCurrentScene != fSceneMap.end()) fCurrentScene->second.RenderGUI();
    else //Render Viewer controls
    {
      //A button to select the current Camera
      ImGui::Text("Cameras");
      ImGui::Separator();
      for(auto cam = fCameras.begin(); cam != fCameras.end(); ++cam) 
      {
        selected = (fCurrentCamera == cam);
        ImGui::Selectable(cam->first.c_str(), &selected);
        if(selected) fCurrentCamera = cam;
      }

      //Render camera controls
      GetCurrentCamera()->render(); 

      //Render overall Viewer controls
      ImGui::Separator();
      ImGui::Text("Viewer Controls");
      ImGui::Separator();
      ImGui::ColorEdit3("Choose a Background", glm::value_ptr(fBackgroundColor));
      //TODO: Am I missing any controls?  Please let me know if you have requests!
    }
    ImGui::End();    

    //Send mouse and keyboard events to the current Camera
    GetCurrentCamera()->update(ioState);

    //TODO: Dont' adjust camera if handled a click here
    if(!ioState.WantCaptureMouse && ImGui::IsMouseClicked(0)) on_click(0, ioState.MousePos.x, ioState.MousePos.y, width, height);

    render(width, height);
  }

  //TODO: The next two functions move to the .h file when this becomes a function template
  ctrl::SceneController& Viewer::MakeScene(const std::string& name, std::shared_ptr<ctrl::ColumnModel> cols, const std::string& fragSrc, const std::string& vertSrc, 
                         std::unique_ptr<SceneConfig>&& config)
  {
    PrepareToAddScene(name);
    auto& scene = fSceneMap.emplace(std::piecewise_construct, std::forward_as_tuple(name),
                      std::forward_as_tuple(fragSrc, vertSrc, cols, std::move(config))).first->second;
    ConfigureNewScene(name, scene, *cols); //lol
    return scene;
  }
 
  ctrl::SceneController& Viewer::MakeScene(const std::string& name, std::shared_ptr<ctrl::ColumnModel> cols, const std::string& fragSrc, const std::string& vertSrc, 
                         const std::string& geomSrc, std::unique_ptr<SceneConfig>&& config)
  {
    PrepareToAddScene(name);
    auto& scene = fSceneMap.emplace(std::piecewise_construct, std::forward_as_tuple(name),
                                    std::forward_as_tuple(fragSrc, vertSrc, geomSrc, cols, std::move(config))).first->second;
    ConfigureNewScene(name, scene, *cols); 
    return scene;
  }

  void Viewer::PrepareToAddScene(const std::string& name)
  {
    auto found = fSceneMap.find(name);
    if(found != fSceneMap.end())
    {
      throw util::GenException("Duplicate Scene Name") << "In mygl::Viewer::MakeScene(), requested scene name " << name << " is already in use.\n";
    }
  }

  void Viewer::ConfigureNewScene(const std::string& name, ctrl::SceneController& scene, ctrl::ColumnModel& cols)
  {
    //Now, tell this Viewer's GUI about the Scene's GUI
  }

  void Viewer::area_realize()
  {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  }

  void Viewer::unrealize()
  {
  }

  void Viewer::render(const int width, const int height) 
  {
    glClearColor(fBackgroundColor.x, fBackgroundColor.y, fBackgroundColor.z, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    assert(GetCurrentCamera() != nullptr);

    for(auto& scenePair: fSceneMap)
    {
      const auto view = GetCurrentCamera()->GetView();
      scenePair.second.Render(view, GetCurrentCamera()->GetPerspective(width, height));
    }
  }
  
  void Viewer::LoadCameras(std::map<std::string, std::unique_ptr<mygl::Camera>>&& cameraToName)
  {
    fCameras = std::move(cameraToName);
    fCurrentCamera = fCameras.begin();
  }

  std::unique_ptr<Camera>& Viewer::GetCurrentCamera()
  {
    if(fCurrentCamera == fCameras.end()) return fDefaultCamera;
    return fCurrentCamera->second;
  }

  bool Viewer::on_click(const int button, const float x, const float y, const int width, const int height)
  {
    if(button != 0) return false; //button 1 is the left mouse button
    
    //TODO: Encapsulate "global" opengl settings into an object that can apply defaults here
    glDisable(GL_BLEND);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f); //TODO: Make sure there is no VisID that maps to this color.  
                                              
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
 
    for(auto& scenePair: fSceneMap)
    {
      const auto view = GetCurrentCamera()->GetView();
 
      //TODO: It would be great to be able to change out this selection algorithm.  
      scenePair.second.RenderSelection(view, GetCurrentCamera()->GetPerspective(width, height));
      //I am not requesting a render here because I want to wait until reacting to the user's selection before rendering.  
    }
    glFlush();
  
    //TODO: Class/struct to encapsulate an opengl drawing state
    glEnable(GL_BLEND);

    //Method for reading pixels taken from http://www.opengl-tutorial.org/miscellaneous/clicking-on-objects/picking-with-an-opengl-hack/
    glFinish(); //Wait for all drawing to finish
    glPixelStorei(GL_PACK_ALIGNMENT, 1); 
    unsigned char color[4];
    glReadPixels(x, height-y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, color); 

    //React to the user's selection.  Ultimately, I want to propagate this to other Viewers, so emit a signal that Viewers 
    //and/or Scenes can react to.  
    //TODO: Selection object like Gtk::TreeView::Selection instead of emitting a signal here?  
    //      I would want a way for multiple Viewers to post events to this Selection object.  
    //fSignalSelection.emit(mygl::VisID(color[0], color[1], color[2])); //, evt->x, evt->y); 
    on_selection(mygl::VisID(color[0], color[1], color[2]));

    return true;
  }

  /*Viewer::SignalSelection Viewer::signal_selection()
  {
    return fSignalSelection;
  }*/

  void Viewer::on_selection(const mygl::VisID id/*, const int x, const int y*/)  
  {
    //Tell Scenes to select chosen object.
    for(auto scenePair = fSceneMap.begin(); scenePair != fSceneMap.end(); ++scenePair) 
    {
      if(scenePair->second.SelectID(id)) fCurrentScene = scenePair; 
    }
  }
}
