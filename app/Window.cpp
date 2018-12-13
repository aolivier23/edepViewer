//File: Window.cpp
//Brief: Displays a ROOT geometry with a List Tree view that shows data about each element.
//       Based heavily on https://developer.gnome.org/gtkmm-tutorial/stable/sec-treeview-examples.html.en
//Author: Andrew Olivier

//The core ImGUI function definitions
#include "imgui.h"

//evd includes
#include "Window.h"

//gl includes
#include "gl/camera/PlaneCam.h"
//#include "gl/objects/Texture2D.cpp"


//Load plugins for drawing from Factory
#include "plugins/Factory.cpp"

//glm includes
#include <glm/gtc/type_ptr.hpp>

//ROOT includes
#include "TDatabasePDG.h"
#include "TParticlePDG.h"
#include "TASImage.h" //For writing images to a file

namespace
{
  //Shortcut for loading all plugins of type BASE from a YAML node's name element.  Throws std::runtime_error if 
  //pluginConfig[name] doesn't exist.
  template <class BASE>
  void loadPlugins(const YAML::Node& pluginConfig, const std::string& name, std::vector<std::unique_ptr<BASE>>& plugins)
  {
    auto& factory = plgn::Factory<BASE>::instance();
    if(pluginConfig[name])
    {
      const auto& config = pluginConfig[name];
      for(auto plugin = config.begin(); plugin != config.end(); ++plugin)
      {
        auto drawer = factory.Get(plugin->first.as<std::string>(), plugin->second);
        if(drawer != nullptr) plugins.push_back(std::move(drawer));
        else std::cerr << "Failed to get " << name << " plugin named " << plugin->first << "(end)\n";
      }
    }
    else throw std::runtime_error("Failed to get an element named "+name+" from config.yaml.\n");
  }
}

namespace evd
{
  Window::Window(std::unique_ptr<YAML::Node>&& config, std::unique_ptr<src::Source>&& source): fConfig(new YAML::Node()),
                 fMaxEventCacheSize(5), fViewer(std::unique_ptr<mygl::Camera>(new mygl::PlaneCam(glm::vec3(0., 0., 1000.), glm::vec3(0., 0., -1.), glm::vec3(0.0, 1.0, 0.0), 10000., 100.)), 10., 10., 10.),
    fSource(), fServices(), fCurrentEvent(std::numeric_limits<int>::min(), std::numeric_limits<int>::min(), "DEFAULT", false)
    //, fPrintTexture(nullptr)
  {
    reconfigure(std::move(config));
    SetSource(std::move(source));
  }

  void Window::reconfigure(std::unique_ptr<YAML::Node>&& config)
  {
    fConfig = std::move(config); //Take ownership of config and manage its' lifetime

    //TODO: Preprocessing for an include directive?
    if(fConfig)
    {
      //fConfig shall be a YAML document that contains a mapping with the keys Global and Event.  These keys 
      //shall be sequences of maps.  An External key is also recognized in a similar format to Global and Event.  
      //Global shall contain all GeoDrawers.
      //Event shall contain all EventDrawers.
      //External shall contain all ExternalDrawers.
      //Each Drawer's configuration map shall have a tag with the c++ type of Drawer it shall configure.
      //TODO: There is supposed to be a YAML TAG structure to handle "native" types.  Can I exploit it here?

      //Load global plugins
      const auto& top = *fConfig;
      const auto& drawers = top["Drawers"];
      ::loadPlugins(drawers, "Global", fGlobalDrawers); //TODO: If this works, replace all other plugin-loading loops

      //Load event plugins
      ::loadPlugins(drawers, "Event", fEventDrawers);
      
      //Load camera config plugins
      ::loadPlugins(drawers, "Camera", fCameraConfigs);

      //Load external plugins
      /*auto& extFactory = plgn::Factory<draw::ExternalDrawer>::instance();
      if(drawers["External"])
      {
        for(YAML::const_iterator plugin = drawers["External"].begin(); plugin != drawers["External"].end(); ++plugin)
        {
          auto drawer = extFactory.Get(plugin->first.as<std::string>(), plugin->second);
          if(drawer != nullptr) fExtDrawers.push_back(std::move(drawer));
          else std::cerr << "Failed to get external plugin named " << plugin->first << "\n";
        }
      }*/
      make_scenes();
    }
  }

  Window::~Window() {}

  //TODO: Stop all threads before calling this!
  void Window::SetSource(std::unique_ptr<src::Source>&& source)
  {
    fSource.reset(source.release());
    assert(fSource);
    //ReadGeo is called when the current file changes, so make sure external drawers are aware of the file change.
    if(fSource)
    { 
      ProcessEvent(true);
      //for(const auto& draw: fExtDrawers) draw->ConnectTree(fSource->fReader);
    }
  }

  void Window::ReadGeo()
  {
    //Load service information
    const auto& serviceConfig = (*fConfig)["Services"]; 
    if(!serviceConfig) std::cerr << "Couldn't find services block.\n";
    const auto& geoConfig = serviceConfig["Geo"];
    if(!geoConfig) std::cerr << "Couldn't find Geo service.\n";

    fServices.fGeometry.reset(new util::Geometry(geoConfig, fSource->Geo())); //TODO: No one can be using the geometry when this happens!
    auto man = fSource->Geo();

    //Next, create threads to do "drawing".  These functions shouldn't modify OpenGL state.  
    for(const auto& drawPtr: fGlobalDrawers) drawPtr->Draw(*man, fServices);
  }

  void Window::ReadEvent()
  { 
    //Now, DrawEvents(), which makes no OpenGL calls, can be run in parallel.  
    const auto& evt = fSource->Event();

    for(const auto& drawer: fEventDrawers) drawer->Draw(evt, fServices);
    for(const auto& config: fCameraConfigs) config->MakeCameras(evt, fServices);

    /*fExternalFuture = std::async(std::launch::async, [this, &evt, &id]()
                                                     {
                                                       for(const auto& drawer: fExtDrawers) drawer->DrawEvent(evt, fViewer, id, fServices);
                                                     });*/
  }
  
  void Window::make_scenes()
  {
    //Configure Geometry Scenes
    for(const auto& drawPtr: fGlobalDrawers) drawPtr->RequestScene(fViewer);    

    //Configure Event Scenes
    for(const auto& drawPtr: fEventDrawers) drawPtr->RequestScene(fViewer);

    //Configure External Scenes
    /*for(const auto& extPtr: fExtDrawers) extPtr->RequestScene(fViewer);*/
  }
  
  void Window::Print(const int width, const int height)
  {
    //I learned to do this from the SOIL library:
    //https://github.com/kbranigan/Simple-OpenGL-Image-Library/blob/master/src/SOIL.c
    auto data = new unsigned char[4*width*height];
    glReadPixels(0, 0, width, height, GL_BGRA, GL_UNSIGNED_BYTE, data); //For some reason, I need to use BGR instead of RGB.  
                                                                        //Maybe PNG-specific: https://stackoverflow.com/questions/5123387/loading-a-bmp-into-an-opengl-textures-switches-the-red-and-blue-colors-c-win

    //Use ROOT's interface to AfterImage for now to keep dependencies down.  
    //Maybe write optional plugins for other libraries in the future?  Maybe libpng which is supposed to 
    //have Emscripten support?
    TASImage screenshot;
    screenshot.FromGLBuffer(data, width, height);
    const auto file = fCurrentEvent.fileName;
    const std::string fileBase = file.substr(file.find_last_of("/")+1, file.find_first_of(".")-1);
    screenshot.WriteImage((fileBase+"_run"+std::to_string(fCurrentEvent.runID)+"_evt"+std::to_string(fCurrentEvent.eventID)+".png").c_str()); 
    //TODO: Let user pick name and image type

    delete[] data; //Free image data now that I am done with it

    //fPrintTexture.reset(new mygl::Texture2D(, , nullptr));
    //TODO: Render to a 3D texture (have to sample each z "layer" individually), and write that to a u3d file?
  }

  void Window::Render(const int width, const int height, const ImGuiIO& ioState)
  {
      //Pop up file selection GUI and call reconfigure()
      /*if(!fConfig) 
      {
        auto file = fChoose.Render(".yaml");
        if(file)
        {
          std::string name(file->GetTitle());
          name += "/";
          name += file->GetName();
          std::ifstream file(name);
          std::unique_ptr<YAML::Node> doc(new YAML::Node(YAML::Load(file))); //TODO: Don't reconfigure() while threads are running!
                                                                             //TODO: Is this problem solved if configuration popups are modal?
          if(doc) reconfigure(std::move(doc));
          else throw std::runtime_error("Syntax error in configuration file "+name+"\n");
        }
      }*/

    //Pop up legend of particle colors used
    auto db = TDatabasePDG::Instance();
    ImGui::Begin("Legend");
    for(auto& pdg: *(fServices.fPDGToColor))
    {
      const auto particle = db->GetParticle(pdg.first);
      std::string name;
      if(particle) name = particle->GetName();
      else name = std::to_string(pdg.first);
          
      ImGui::ColorEdit3(name.c_str(), glm::value_ptr(pdg.second), ImGuiColorEditFlags_NoInputs);
    }
    ImGui::End();

    fViewer.Render(width, height, ioState);
  }

  void Window::ProcessEvent(const bool forceGeo)
  {
    fEventCache.push(std::async(std::launch::async,
                                [this, forceGeo]()
                                {
                                  const auto meta = fSource->Next();
                                  if(meta.newFile || forceGeo) ReadGeo();
                                  ReadEvent();
                                  return meta;
                                }));
  }

  void Window::ProcessEvent(const int run, const int event)
  {
    fEventCache.push(std::async(std::launch::async,
                                [this, run, event]()
                                {
                                  const auto meta = fSource->GoTo(run, event);
                                  if(meta.newFile) ReadGeo();
                                  ReadEvent();
                                  return meta;
                                }));
  }

  //fNextEvent must be valid before calling this function
  void Window::LoadNextEvent()
  {
    //Any exceptions from the thread where fNextEvent was "created" will be thrown when I 
    //call fNextEvent.get().  I particularly want to react to no_more_files exceptions.  
    //If I don't get a next event, don't load anything.
    const auto meta = fEventCache.front().get();
    fEventCache.pop(); //Now that we're displaying this event, it's no longer in the cache of events to display in the future
                       //TODO: Move current event to previous event position
    fCurrentEvent = meta; //Assignment on a separate line because I'm afraid of meta getting assigned when fNextEvent throws
    mygl::VisID id(0, 0, 0);
    if(fCurrentEvent.newFile)
    {
      for(const auto& geo: fGlobalDrawers) geo->UpdateScene(id);
    }
    for(const auto& drawer: fEventDrawers) drawer->UpdateScene(id);
    
    std::map<std::string, std::unique_ptr<mygl::Camera>> cameras;
    for(const auto& config: fCameraConfigs) config->AppendCameras(cameras);
    fViewer.LoadCameras(std::move(cameras));
    //TODO: UpdateScene() for ExternalDrawers as well
  }

  void Window::ClearCache()
  {
    for(auto& geo: fGlobalDrawers) geo->Clear();
    for(auto& evt: fEventDrawers) evt->Clear();
    for(auto& cam: fCameraConfigs) cam->Clear();

    fEventCache = std::queue<std::future<src::Source::metadata>>();
  }

  std::future<src::Source::metadata>& Window::NextEventStatus()
  {
    assert(!fEventCache.empty());
    return fEventCache.front();
  }

  std::future<src::Source::metadata>& Window::LastEventStatus()
  {
    assert(!fEventCache.empty());
    return fEventCache.back();
  }

  size_t Window::EventCacheSize() const
  {
    return fEventCache.size();
  }

  size_t Window::MaxEventCacheSize() const
  {
    return fMaxEventCacheSize;
  }

  src::Source::metadata Window::CurrentEvent() const
  {
    return fCurrentEvent;
  }
}
