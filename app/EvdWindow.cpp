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

//local includes
#include "app/Source.h"

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
        else std::cerr << "Failed to get event plugin named " << plugin->first << "(end)\n";
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
    fSource(), fServices(), fIsWaiting(true), fHasNewGeom(true)//, fPrintTexture(nullptr)
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
      //auto& geoFactory = plgn::Factory<draw::GeoDrawerBase>::instance();

      const auto& top = *fConfig;
      const auto& drawers = top["Drawers"];
      ::loadPlugins(drawers, "Global", fGlobalDrawers); //TODO: If this works, replace all other plugin-loading loops
      /*if(drawers["Global"])
      {
        for(auto plugin = drawers["Global"].begin(); plugin != drawers["Global"].end(); ++plugin)
        {
          auto drawer = geoFactory.Get(plugin->first.as<std::string>(), plugin->second);
          if(drawer != nullptr) fGlobalDrawers.push_back(std::move(drawer));
          else std::cerr << "Failed to get global plugin named " << plugin->first << "(end)\n";
        }
      }
      else throw std::runtime_error("Failed to get an element named Global from config.yaml.\n");*/

      //Load event plugins
      /*auto& evtFactory = plgn::Factory<draw::EventDrawerBase>::instance();
      if(drawers["Event"])
      {
        const auto& eventConfig = drawers["Event"];
        for(auto plugin = eventConfig.begin(); plugin != eventConfig.end(); ++plugin)
        {
          auto drawer = evtFactory.Get(plugin->first.as<std::string>(), plugin->second);
          if(drawer != nullptr) fEventDrawers.push_back(std::move(drawer));
          else std::cerr << "Failed to get event plugin named " << plugin->first << "(end)\n";
        }
      }
      else throw std::runtime_error("Failed to get an element named Event from config.yaml.\n");

      //Load camera config plugins
      auto camFactory = plgn::Factory<draw::CameraConfigBase>::instance();
      if(drawers["Camera"])
      {
        const auto& camConfig = drawers["Camera"];
        for(auto plugin = camConfig.begin(); plugin != cam
      }

      //Load external plugins
      auto& extFactory = plgn::Factory<draw::ExternalDrawer>::instance();
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

  //TODO: Maybe defer setting TGeoManager in Source to NextFile() and call NextFile() here.  
  void EvdWindow::SetSource(std::unique_ptr<src::Source>&& source)
  {
    fSource.reset(source.release());
    //ReadGeo is called when the current file changes, so make sure external drawers are aware of the file change.
    //TODO: I should probably relabel this FileChange() and/or make the negotiation with Source a state machine.
    //TODO: Rethink how Source interacts with ExternalDrawers.  
    if(fSource)
    { 
      //for(const auto& draw: fExtDrawers) draw->ConnectTree(fSource->fReader);
      fSource->Next();
      fNextEvent = std::async(std::launch::async, 
                              [this]()
                              {
                                ReadGeo();
                                ReadEvent();
                              });
    }
  }

  void EvdWindow::ReadGeo()
  {
    fIsWaiting = true; //Go to the waiting state if not already there
    fHasNewGeom = true;
    
    //Load service information
    const auto& serviceConfig = (*fConfig)["Services"]; 
    if(!serviceConfig) std::cerr << "Couldn't find services block.\n";
    const auto& geoConfig = serviceConfig["Geo"];
    if(!geoConfig) std::cerr << "Couldn't find Geo service.\n";

    fServices.fGeometry.reset(new util::Geometry(geoConfig, fSource->Geo()));
    auto man = fSource->Geo();

    //Next, create threads to do "drawing".  These functions shouldn't modify OpenGL state.  
    for(const auto& drawPtr: fGlobalDrawers) drawPtr->Draw(*man, fServices);

    std::cout << "Started drawing the geometry.\n";
  }

  void EvdWindow::ReadEvent()
  { 
    std::cout << "Going to next event.\n";
    fIsWaiting = true; //Go to the waiting state if not already there
                       //TODO: A "real" state machine would call some function that implicitly makes the transition to a new state
    //TODO: Rewrite this as just a loop over calling fEventDrawers' and fExtDrawers' Draw() functions.
    //Now, DrawEvents(), which makes no OpenGL calls, can be run in parallel.  
    /*const auto& evt = fSource->Event();

    fEventFuture = std::async(std::launch::async, [this, &evt, &id]() 
                                                  {
                                                    for(const auto& drawPts: fEventDrawers) drawPts->DrawEvent(evt, fViewer, id, fServices);
                                                  });

    fExternalFuture = std::async(std::launch::async, [this, &evt, &id]()
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
    //TODO: Put fEventDrawers back when fGlobalDrawers works
    /*for(const auto& drawPtr: fEventDrawers) drawPtr->RequestScene(fViewer);

    //Configure External Scenes
    for(const auto& extPtr: fExtDrawers) extPtr->RequestScene(fViewer);*/
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
    const auto file = fSource->GetFile();
    const std::string fileBase = file.substr(file.find_last_of("/")+1, file.find_first_of(".")-1);
    screenshot.WriteImage((fileBase+"_run"+std::to_string(fSource->RunID())+"_evt"+std::to_string(fSource->EventID())+".png").c_str()); 
    //TODO: Let user pick name and image type

    delete[] data; //Free image data now that I am done with it

    //fPrintTexture.reset(new mygl::Texture2D(, , nullptr));
    //TODO: Render to a 3D texture (have to sample each z "layer" individually), and write that to a u3d file?
  }

  void EvdWindow::Render(const int width, const int height, const ImGuiIO& ioState)
  {
    //Render Viewer if not currently loading an event.  Otherwise, render a splash screen with a progress bar.  
    if(fIsWaiting) //If in the "waiting for event processing" state
    {
      //TODO: Just clear background in one place
      glClearColor(0.0, 0.0, 0.0, 1.0);
      glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

      //Check whether all Drawers have finished
      std::cout << "Threads are running!  Popping up progress bar...\n";
      ImGui::Begin("Loading");
      ImGui::Text("Loading, next event and/or file...");
      //ImGui::ProgressBar((geoPos)/1., ImVec2(0.0f, 0.0f));
      //if(geoReady) ImGui::Text("Geometry ready!");
      //if(eventReady) ImGui::Text("Edep-sim ready!");
      //if(extReady) ImGui::Text("External ready!");
      ImGui::End();

      if(fNextEvent.wait_for(std::chrono::milliseconds(10)) == std::future_status::ready) 
      {
        mygl::VisID id(0, 0, 0);
        if(fHasNewGeom)
        {
          for(const auto& geo: fGlobalDrawers) geo->UpdateScene(id); //TODO: I don't always want to call UpdateScene for 
          fHasNewGeom = false;
        }
                                                                   //      the geometry when the event drawers are ready.
        //TODO: UpdateScene for external drawers, event drawers, and camera config as well.

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
        //ImGui::SetNextWindowBgAlpha(0.3f); // Transparent background
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

        RenderControlBar(width, height); //TODO: If RenderControlBar did something, return immediately (or else check threads)

        //TODO: Allow Drawers to Render() here?  Really, I think I want to move the PDGToColor service to a 
        //      more generic color mapping service, implement named service instantiations, and Render() 
        //      services so that they can use IMGUI.   
        //for(const auto& drawer: fExtDrawers) drawer->Render();

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

      /*int entry = fSource->Entry();
      if(ImGui::InputInt("TTree Entry", &entry))
      {
        goto_event(entry);
      }*/
      int ids[] = {fSource->RunID(), fSource->EventID()};
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

  void EvdWindow::goto_event(const int evt)
  {
    std::cout << "Calling function EvdWindow::goto_event()\n";
    if(fSource->GoTo(evt)) 
    {
      ReadEvent();
    }
    else std::cerr << "Failed to get event " << evt << " from file " << fSource->GetFile() << "\n";
  }

  void EvdWindow::goto_id(const int run, const int evt)
  {
    if(fSource->GoTo(run, evt))
    {
      ReadEvent();
    }
    else std::cerr << "Failed to get event (" << run << ", " << evt << ") from file " << fSource->GetFile() << "\n";
  }

  void EvdWindow::next_event()
  {
    std::cout << "Calling function EvdWindow::next_event()\n";
    if(!fSource->Next()) //If this is the end of the current file
    {
      //TODO: NextFile() and/or Next() in a thread?
      if(!fSource->NextFile()) //If this is the end of the last file in the Source
      {
        std::cerr << "Reached last event in input files.\n";
      }
      else 
      {
        std::cout << "Reading from file " << fSource->GetFile() << "\n";
        fNextEvent = std::async(std::launch::async, 
                                [this]()
                                {
                                  ReadGeo();
                                  ReadEvent(); 
                                });
      }
    }
    else 
    {
      std::cout << "Reading from file " << fSource->GetFile() << "\n";
      fNextEvent = std::async(std::launch::async, [this](){ ReadEvent(); }); 
    }
  }
}


