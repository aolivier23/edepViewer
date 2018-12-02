//File: EvdWindow.cpp
//Brief: Displays a ROOT geometry with a List Tree view that shows data about each element.
//       Based heavily on https://developer.gnome.org/gtkmm-tutorial/stable/sec-treeview-examples.html.en
//Author: Andrew Olivier

//The core ImGUI function definitions
#include "imgui.h"

//evd includes
#include "EvdWindow.h"

//gl includes
#include "gl/camera/PlaneCam.h"
//#include "gl/objects/Texture2D.cpp"

//Load plugins for drawing from Factory
#include "plugins/Factory.cpp"

//glm includes
#include <glm/gtc/type_ptr.hpp>

//ROOT includes
#include "TGeoManager.h"
#include "TGeoNode.h"
#include "TGeoVolume.h"
#include "TFile.h"
#include "TLorentzVector.h"
#include "TBuffer3D.h"
#include "TDatabasePDG.h"
#include "TParticlePDG.h"
#include "TASImage.h" //For writing images to a file

//c++ includes
#include <regex>

namespace
{
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

namespace mygl
{
  EvdWindow::EvdWindow(): fConfig(new YAML::Node()),
    fViewer(std::unique_ptr<mygl::Camera>(new mygl::PlaneCam(glm::vec3(0., 0., 1000.), glm::vec3(0., 0., -1.), glm::vec3(0.0, 1.0, 0.0), 10000., 100.)),
            10., 10., 10.),
    fSource(), fServices(), fIsWaiting(true), fCurrentEvent(std::numeric_limits<int>::min(), std::numeric_limits<int>::min(), "DEFAULT", false) 
    //, fPrintTexture(nullptr)
  {
  }

  void EvdWindow::reconfigure(std::unique_ptr<YAML::Node>&& config)
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

  EvdWindow::~EvdWindow() {}

  //TODO: Stop all threads before calling this!
  void EvdWindow::SetSource(std::unique_ptr<src::Source>&& source)
  {
    fSource.reset(source.release());
    //ReadGeo is called when the current file changes, so make sure external drawers are aware of the file change.
    //TODO: I should probably relabel this FileChange() and/or make the negotiation with Source a state machine.
    //TODO: Rethink how Source interacts with ExternalDrawers.  
    if(fSource)
    { 
      std::cout << "Setting future because of a new Source.\n";
      fIsWaiting = true;
      //TODO: Clear all event caches here
      //for(const auto& draw: fExtDrawers) draw->ConnectTree(fSource->fReader);
      fNextEvent = std::async(std::launch::async,
                              [this]()
                              {
                                auto meta = fSource->Next();
                                ReadGeo(); 
                                ReadEvent();
                                return meta;
                              });
    }
  }

  void EvdWindow::ReadGeo()
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

  void EvdWindow::ReadEvent()
  { 
    //TODO: Rewrite this as just a loop over calling fEventDrawers' and fExtDrawers' Draw() functions.
    //Now, DrawEvents(), which makes no OpenGL calls, can be run in parallel.  
    const auto& evt = fSource->Event();

    for(const auto& drawer: fEventDrawers) drawer->Draw(evt, fServices);
    for(const auto& config: fCameraConfigs) config->MakeCameras(evt, fServices);

    /*fExternalFuture = std::async(std::launch::async, [this, &evt, &id]()
                                                     {
                                                       for(const auto& drawer: fExtDrawers) drawer->DrawEvent(evt, fViewer, id, fServices);
                                                     });*/
  }
  
  void EvdWindow::make_scenes()
  {
    std::cout << "Calling function EvdWindow::make_scenes()\n";

    //Configure Geometry Scenes
    for(const auto& drawPtr: fGlobalDrawers) drawPtr->RequestScene(fViewer);    

    //Configure Event Scenes
    for(const auto& drawPtr: fEventDrawers) drawPtr->RequestScene(fViewer);

    //Configure External Scenes
    /*for(const auto& extPtr: fExtDrawers) extPtr->RequestScene(fViewer);*/
  }
  
  void EvdWindow::Print(const int width, const int height)
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

  void EvdWindow::Render(const int width, const int height, const ImGuiIO& ioState)
  {
    //Render Viewer if not currently loading an event.  Otherwise, render a splash screen with a progress bar.  
    if(fIsWaiting) //If in the "waiting for event processing" state
                   //TODO: Re-organize event processing yet again so that std::future<>::wait_for() is the only 
                   //      state I need to query.
    {
      //TODO: Just clear background in one place
      glClearColor(0.0, 0.0, 0.0, 1.0);
      glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

      //Check whether all Drawers have finished
      ImGui::Begin("Loading"); //TODO: Put this window off to the side, but keep it obvious
      ImGui::Text("Loading, next event and/or file...");
      ImGui::End();

      if(fNextEvent.wait_for(std::chrono::milliseconds(10)) == std::future_status::ready) 
      {
        std::cout << "Event is ready!\n";
        try //Any exceptions from the thread where fNextEvent was "created" will be thrown when I 
            //call fNextEvent.get().  I particularly want to react to no_more_files exceptions.  
            //If I don't get a next event, don't load anything.
        {
          const auto meta = fNextEvent.get();
          fCurrentEvent = meta; //TODO: If meta is guaranteed never to be assigned if std::future<>::get() throws, then remove this line
          mygl::VisID id(0, 0, 0);
          if(fCurrentEvent.newFile)
          {
            for(const auto& geo: fGlobalDrawers) geo->UpdateScene(id);
          }
          for(const auto& drawer: fEventDrawers) drawer->UpdateScene(id);
        
          std::map<std::string, std::unique_ptr<mygl::Camera>> cameras;
          for(const auto& config: fCameraConfigs) config->AppendCameras(cameras);
          fViewer.LoadCameras(std::move(cameras));
        }
        catch(const src::Source::no_more_files& e)
        {
          std::cerr << e.what() << "\n"; //TODO: Put error message into a modal window
          //TODO: Reload previous event?
        }

        //TODO: UpdateScene() for ExternalDrawers as well

        fIsWaiting = false;
      }
    }
    else //Otherwise, we can render the current event.  We might still be processing future events in another thread.  
    {
      //Pop up file selection GUI and call reconfigure()
      if(!fConfig) 
      {
        auto file = fChoose.Render(".yaml");
        if(file)
        {
          std::string name(file->GetTitle());
          name += "/";
          name += file->GetName();
          std::ifstream file(name);
          std::unique_ptr<YAML::Node> doc(new YAML::Node(YAML::Load(file)));
          if(doc) reconfigure(std::move(doc));
          else throw std::runtime_error("Syntax error in configuration file "+name+"\n");
        }
      }

      //Pop up file selection GUI and call SetSource()
      if(!fSource)
      {
        choose_file();
      }

      if(fSource && fConfig)
      {
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

        RenderControlBar(width, height);
        fViewer.Render(width, height, ioState);
      }
    }
  }

  void EvdWindow::RenderControlBar(const int width, const int height)
  {
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x, 0.f), ImGuiCond_Always, ImVec2(1.0f, 1.0));
    ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, 20.f));
    ImGui::Begin("Control Bar", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    {
      if(ImGui::Button("Print"))
      {
        Print(width, height); 
      } 
      ImGui::SameLine();

      int ids[] = {fCurrentEvent.runID, fCurrentEvent.eventID}; //TODO: current event is different from next event
      if(ImGui::InputInt2("(Run, Event)", ids, ImGuiInputTextFlags_EnterReturnsTrue))
      {
        goto_id(ids[0], ids[1]);
      }
      ImGui::SameLine();

      if(ImGui::Button("Next")) next_event(); //TODO: Make goto_event() able to detect the end of a file like next_event()
      ImGui::SameLine();
      if(ImGui::Button("Reload")) ReadEvent(); //fNextEvent = std::async(std::launch::async, this->ReadEvent);
      ImGui::SameLine();
      if(ImGui::Button("File")) fSource.reset(nullptr); //Source reading has already happened, so reset fSource to cause a file chooser 
                                                        //GUI to pop up in next loop.
                                                        //TODO: Stop all threads first?
      //TODO: Status of event pre-loading?
    }
    ImGui::End();
  }

  void EvdWindow::choose_file()
  {
    //Pop up file selection GUI and call SetSource()
    auto file = fChoose.Render(".root");
    if(file)
    {
      std::string name(file->GetTitle());
      name += "/";
      name += file->GetName();
      SetSource(std::unique_ptr<src::Source>(new src::Source(name)));
    }
  }

  void EvdWindow::goto_id(const int run, const int evt)
  {
    fIsWaiting = true;
    fNextEvent = std::async(std::launch::async,
                               [this, &run, &evt]()
                               {
                                 const auto meta = fSource->GoTo(run, evt);
                                 ReadEvent();
                                 //TODO: Clear all Drawers' caches after loading this event?
                                 return meta;
                               });
  }

  void EvdWindow::next_event()
  {
    fIsWaiting = true;
    fNextEvent = std::async(std::launch::async,
                            [this]()
                            {
                              const auto meta = fSource->Next();
                              if(meta.newFile) ReadGeo();
                              ReadEvent();
                              return meta;
                            });
  }
}


