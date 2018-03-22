//File: Viewer.h
//Brief: Creates an area for opengl drawing in a Gtk Widget.  Expects to be given constructor information for Drawables by 
//       external code, then draws those Drawables.  User can remove Drawables by VisID.  Drawables are managed by GLScenes.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//c++ includes
#include <memory> //for std::unique_ptr
#include <giomm/resource.h> //for Glib::RefPointer?
#include <type_traits> //for std::true_type and std::false_type

//local includes
#include "gl/Viewer.h"
#include "gl/UserCut.h"
#include "gl/camera/Camera.h"

//imgui includes
#include "imgui.h"

//c++ includes
#include <iostream>
#include <sstream>
#include <utility> //For std::piecewise_construct shenanigans
#include <tuple> //For std::forward_as_tuple

#include "glad/include/glad/glad.h"

namespace mygl
{
  Viewer::Viewer(std::unique_ptr<Camera>&& cam, const float xPerPixel, const float yPerPixel, const float zPerPixel):
                fSceneMap(), 
                fCameras(), //fCameras(1, cam), fCurrentCamera(fCameras.begin()),
                fXPerPixel(xPerPixel), fYPerPixel(yPerPixel), fZPerPixel(zPerPixel)
  {
    fCameras.emplace_back(cam.release());
    fCurrentCamera = fCameras.begin();
    //Setup control widgets
    //TODO: Move background color to a central location for future applications -> Viewer-independent configuration tab 
    /*fNotebook.set_hexpand(false);
    fNotebook.set_scrollable();
    fControl.pack_start(fBackColorLabel, Gtk::PACK_SHRINK);
    fControl.pack_start(fBackgroundButton, Gtk::PACK_SHRINK);
    fBackgroundButton.signal_color_set().connect(sigc::mem_fun(*this, &Viewer::set_background));

    //Setup camera selection GUI.  This really needs to be its' own tab in a Gtk::Stack
    fCameraSwitch.set_stack(fCameras);
    fCameras.add(*fCurrentCamera, "Default", "Default");
    fCurrentCamera->ConnectSignals(fArea);

    fControl.pack_start(fCameraSwitch, Gtk::PACK_SHRINK);
    fControl.pack_start(fCameras, Gtk::PACK_SHRINK);

    fNotebook.append_page(fControl, "Viewer");

    //Setup GLArea
    fArea.set_hexpand(true);

    pack1(fArea, Gtk::EXPAND);        
    pack2(fNotebook, Gtk::SHRINK); 

    add_events(Gdk::SCROLL_MASK | Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK
                     | Gdk::KEY_PRESS_MASK | Gdk::KEY_RELEASE_MASK | Gdk::POINTER_MOTION_MASK);
    fArea.add_events(Gdk::SCROLL_MASK | Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK 
                     | Gdk::KEY_PRESS_MASK | Gdk::KEY_RELEASE_MASK | Gdk::POINTER_MOTION_MASK);
    fArea.set_can_focus(true); //This allows fArea to get keyboard events until another widget grabs focus
    fArea.grab_focus();
    
    fArea.set_required_version(3, 3);

    //Looks like window layout hints (ROOT's TGLayoutHints?)
    //TODO: Should the Window including this object call these?
    //fArea.set_hexpand(true);
    fArea.set_vexpand(true);
    fArea.set_auto_render(true);
    
    //GLArea signals
    //TODO: Confirm that these work.  I am now overriding these methods directly for Gtk::GLArea
    fArea.signal_realize().connect(sigc::mem_fun(*this, &Viewer::area_realize), false);
    fArea.signal_unrealize().connect(sigc::mem_fun(*this, &Viewer::unrealize), false);
    fArea.signal_render().connect(sigc::mem_fun(*this, &Viewer::render), false);
    fArea.signal_motion_notify_event().connect(sigc::mem_fun(*this, &Viewer::my_motion_notify_event));
    fArea.signal_button_release_event().connect(sigc::mem_fun(*this, &Viewer::on_click), false);*/

    //Make sure this Viewer reacts to its' own selection signal.  Other viewers can also connect via the public interface.
    fSignalSelection.connect(sigc::mem_fun(*this, &Viewer::on_selection));

    //Configure opengl
    //fArea.set_has_depth_buffer(true);

    //fCameras.property_visible_child().signal_changed().connect(sigc::mem_fun(*this, &Viewer::camera_change));
    area_realize();
  }

  Viewer::~Viewer() {}

  //TODO: Rename this and/or render() so that the differences between them are more clear. 
  void Viewer::Render(const int width, const int height, const ImGuiIO& ioState)
  {
    ImGui::Begin("Viewer"); //TODO: Viewer name
    (*fCurrentCamera)->render(ioState);
    //TODO: Render() current scene
    fSceneMap.begin()->second.RenderGUI();
    //TODO: Render() view controls
    ImGui::End();    

    //TODO: Dont' adjust camera if handled a click here
    if(!ioState.WantCaptureMouse && ImGui::IsMouseClicked(0)) on_click(0, ioState.MousePos.x, ioState.MousePos.y, width, height);

    render(width, height);
  }

  //TODO: Return some sort of TreeView configuration
  void Viewer::MakeScene(const std::string& name, std::shared_ptr<mygl::ColRecord> cols, const std::string& fragSrc, const std::string& vertSrc)
  {
    PrepareToAddScene(name);
    ConfigureNewScene(name, fSceneMap.emplace(std::piecewise_construct, std::forward_as_tuple(name), 
                      std::forward_as_tuple(name, fragSrc, vertSrc, cols)).first->second, *cols); //lol
  }
 
  //TODO: Return some sort of TreeView configuration
  void Viewer::MakeScene(const std::string& name, std::shared_ptr<mygl::ColRecord> cols, const std::string& fragSrc, const std::string& vertSrc, 
                                   const std::string& geomSrc)
  {
    PrepareToAddScene(name);
    ConfigureNewScene(name, fSceneMap.emplace(std::piecewise_construct, std::forward_as_tuple(name),
                      std::forward_as_tuple(name, fragSrc, vertSrc, geomSrc, cols)).first->second, *cols); //lol
  }

  void Viewer::PrepareToAddScene(const std::string& name)
  {
    auto found = fSceneMap.find(name);
    if(found != fSceneMap.end())
    {
      throw util::GenException("Duplicate Scene Name") << "In mygl::Viewer::MakeScene(), requested scene name " << name << " is already in use.\n";
    }
  }

  //TODO: Return some sort of TreeView configuration
  void Viewer::ConfigureNewScene(const std::string& name, mygl::Scene& scene, mygl::ColRecord& cols)
  {
    //Now, tell this Viewer's GUI about the Scene's GUI
    //auto& treeView = scene.fTreeView;
    //treeView.set_enable_search(true);

    /*fScrolls.emplace_back();
    auto& scroll = fScrolls.back().second;
    scroll.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC); //Make sure to expand scrollable area when more 
                                                                     //rows are added.
    scroll.add(treeView);
    auto& box = fScrolls.back().first;
    box = Gtk::Box(Gtk::ORIENTATION_VERTICAL);
    box.pack_start(scene.fCutBar, Gtk::PACK_SHRINK);
    box.pack_end(scroll);
    fNotebook.append_page(box, name);
    fNotebook.show_all_children();
    
    return treeView;*/
  }

  void Viewer::area_realize()
  {
    //Quick GUI interlude in a convenient place
    //set_position(get_width()*0.8); //Set default relative size of GLArea

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  }

  void Viewer::unrealize()
  {
  }

  void Viewer::render(const int width, const int height) 
  {
    if(*fCurrentCamera == nullptr) std::cerr << "fCurrentCamera is not set!\n";

    for(auto& scenePair: fSceneMap)
    {
      const auto view = (*fCurrentCamera)->GetView();

      scenePair.second.Render(view, glm::scale((*fCurrentCamera)->GetPerspective(width, height), 
                                               glm::vec3(1.f/fXPerPixel, 1.f/fYPerPixel, 1.f/fZPerPixel)));
    }
  }
  
  void Viewer::set_background()
  {
    /*fBackgroundColor = fBackgroundButton.get_rgba();
    fBackgroundColor.set_alpha(0.0);
    //ignore alpha
    fArea.queue_render();*/
  }

  void Viewer::AddCamera(const std::string& name, std::unique_ptr<Camera>&& camera)
  {
    std::cout << "Called mygl::Viewer::AddCamera()\n";
    //fCameras.add(*(camera.release()), name, name);
    fCameras.emplace_back(camera.release());
  }

  void Viewer::camera_change()
  {
    std::cout << "Called mygl::Viewer::camera_change()\n";
    //fCurrentCamera = ((Camera*)(fCameras.get_visible_child()));
    //fCurrentCamera->ConnectSignals(fArea);
    //fArea.queue_render();
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
      const auto view = (*fCurrentCamera)->GetView();
 
      //TODO: It would be great to be able to change out this selection algorithm.  
      scenePair.second.RenderSelection(view, glm::scale((*fCurrentCamera)->GetPerspective(width, height),
                                       glm::vec3(1.f/fXPerPixel, 1.f/fYPerPixel, 1.f/fZPerPixel)));
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

  Viewer::SignalSelection Viewer::signal_selection()
  {
    return fSignalSelection;
  }

  void Viewer::on_selection(const mygl::VisID id/*, const int x, const int y*/)  
  {
    //Tell Scenes to select chosen object.
    for(auto& scenePair: fSceneMap) 
    {
      if(scenePair.second.SelectID(id)); //fNotebook.set_current_page(fNotebook.page_num(
                                        //                           *(scenePair.second.fTreeView.get_parent()->get_parent())));
    }
  }

  void Viewer::RemoveAll(const std::string& sceneName)
  {
    //TODO: Error handling when sceneName is not found?  Really, this, along with AddDrawable, reveals that my Viewer/Scene system 
    //      would benefit from a redesign.  
    auto found = fSceneMap.find(sceneName);
    if(found != fSceneMap.end()) found->second.RemoveAll();
  }
}
