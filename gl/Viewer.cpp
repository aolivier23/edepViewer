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

//gtk includes
#include <gtkmm.h>

//c++ includes
#include <iostream>
#include <sstream>
#include <utility> //For std::piecewise_construct shenanigans
#include <tuple> //For std::forward_as_tuple

#include "glad/include/glad/glad.h"

namespace mygl
{
  Viewer::Viewer(std::unique_ptr<Camera>&& cam, const Gdk::RGBA& background, const float xPerPixel, const float yPerPixel, const float zPerPixel):
                Gtk::Paned(Gtk::ORIENTATION_HORIZONTAL), fSceneMap(), fArea(), fBackgroundColor(background), fScrolls(), 
                fControl(Gtk::ORIENTATION_VERTICAL), fBackColorLabel("Background Color"), fCameras(), fCameraSwitch(), fCurrentCamera(cam.release()),
                fBackgroundButton(background), fXPerPixel(xPerPixel), fYPerPixel(yPerPixel), fZPerPixel(zPerPixel)
  {
    //Setup control widgets
    //TODO: Move background color to a central location for future applications -> Viewer-independent configuration tab 
    fNotebook.set_hexpand(false);
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

    //Configure opengl
    //fArea.set_has_depth_buffer(true);

    show_all_children();
    fCameras.property_visible_child().signal_changed().connect(sigc::mem_fun(*this, &Viewer::camera_change));
  }

  Viewer::~Viewer() {}

  Gtk::TreeView& Viewer::MakeScene(const std::string& name, mygl::ColRecord& cols, const std::string& fragSrc, const std::string& vertSrc)
  {
    PrepareToAddScene(name);
    return ConfigureNewScene(name, fSceneMap.emplace(std::piecewise_construct, std::forward_as_tuple(name), 
                                    std::forward_as_tuple(name, fragSrc, vertSrc, cols)).first->second); //lol
  }

  Gtk::TreeView& Viewer::MakeScene(const std::string& name, mygl::ColRecord& cols, const std::string& fragSrc, const std::string& vertSrc, 
                                   const std::string& geomSrc)
  {
    PrepareToAddScene(name);
    return ConfigureNewScene(name, fSceneMap.emplace(std::piecewise_construct, std::forward_as_tuple(name),
                                    std::forward_as_tuple(name, fragSrc, vertSrc, geomSrc, cols)).first->second); //lol
  }

  void Viewer::PrepareToAddScene(const std::string& name)
  {
    auto found = fSceneMap.find(name);
    if(found != fSceneMap.end())
    {
      throw util::GenException("Duplicate Scene Name") << "In mygl::Viewer::MakeScene(), requested scene name " << name << " is already in use.\n";
    }

    fArea.throw_if_error();
    fArea.make_current();
  }

  Gtk::TreeView& Viewer::ConfigureNewScene(const std::string& name, mygl::Scene& scene)
  {
    //Now, tell this Viewer's GUI about the Scene's GUI
    auto& treeView = scene.fTreeView;
    treeView.set_enable_search(true);
    fScrolls.emplace_back();
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
    
    return treeView;
  }

  void Viewer::area_realize()
  {
    //Quick GUI interlude in a convenient place
    set_position(get_width()*0.8); //Set default relative size of GLArea

    fArea.make_current();
    try
    {
      fArea.throw_if_error();
      if(!gladLoadGL())
      {
        std::cerr << "Failed to load opengl extensions with glad.\n";
      }

      glViewport(0, 0, fArea.get_allocated_width(), fArea.get_allocated_height());

      //enable depth testing
      //glEnable(GL_DEPTH_TEST);
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glLineWidth(3.0f); //TODO: User parameter
    }
    catch(const Gdk::GLError& err)
    {
      std::cerr << "Caught exception in Viewer::realize():\n"
                << err.domain() << "-" << err.code() << "-" << err.what() << "\n";
      //TODO: Should I rethrow?
    }
  }

  void Viewer::unrealize()
  {
    fArea.make_current();
    try
    {
      fArea.throw_if_error();
    }
    catch(const Gdk::GLError& err)
    {
      std::cerr << "Caught exception in Viewer::unrealize():\n"
                << err.domain() << "-" << err.code() << "-" << err.what() << "\n";
      //TODO: Should I rethrow?
    }
  }

  bool Viewer::render(const Glib::RefPtr<Gdk::GLContext>& /*context*/) 
  {
    fArea.make_current();
    try
    {
      fArea.throw_if_error();

      glClearColor(fBackgroundColor.get_red(), fBackgroundColor.get_green(), fBackgroundColor.get_blue(), 1.0f); //TODO: Allow user to set background color
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      if(fCurrentCamera == nullptr) std::cerr << "fCurrentCamera is not set!\n";

      for(auto& scenePair: fSceneMap)
      {
        const auto view = fCurrentCamera->GetView();

        scenePair.second.Render(view, glm::scale(fCurrentCamera->GetPerspective(fArea.get_allocated_width(), fArea.get_allocated_height()), 
                                                 glm::vec3(1.f/fXPerPixel, 1.f/fYPerPixel, 1.f/fZPerPixel)));
      }
      glFlush();

      //Force continuous rendering.  
      fArea.queue_render(); 

      return false; 
    }
    catch(const Gdk::GLError& err)
    {
      std::cerr << "Caught exception in Viewer::render():\n"
                << err.domain() << "-" << err.code() << "-" << err.what() << "\n";
      return false;
    }
  }
  
  bool Viewer::my_motion_notify_event(GdkEventMotion* /*evt*/)
  {
    fArea.grab_focus();
    return false;
  }
  
  void Viewer::set_background()
  {
    fBackgroundColor = fBackgroundButton.get_rgba();
    fBackgroundColor.set_alpha(0.0);
    //ignore alpha
    fArea.queue_render();
  }

  void Viewer::AddCamera(const std::string& name, std::unique_ptr<Camera>&& camera)
  {
    std::cout << "Called mygl::Viewer::AddCamera()\n";
    fCameras.add(*(camera.release()), name, name);
  }

  void Viewer::camera_change()
  {
    std::cout << "Called mygl::Viewer::camera_change()\n";
    fCurrentCamera = ((Camera*)(fCameras.get_visible_child()));
    fCurrentCamera->ConnectSignals(fArea);
    fArea.queue_render();
  }
}
