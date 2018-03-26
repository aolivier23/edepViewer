//File: EvdWindow.cpp
//Brief: Displays a ROOT geometry with a List Tree view that shows data about each element.
//       Based heavily on https://developer.gnome.org/gtkmm-tutorial/stable/sec-treeview-examples.html.en
//Author: Andrew Olivier

//The core ImGUI function definitions
#include "imgui.h"

//evd includes
#include "EvdWindow.h"

//gl includes
#include "gl/model/PolyMesh.h"
#include "gl/model/Path.h"
#include "gl/model/Grid.h"
#include "gl/model/Point.h"
#include "gl/camera/PlaneCam.h"

//Load plugins for drawing from Factory
#include "plugins/Factory.cpp"

//glm includes
#include <glm/gtc/type_ptr.hpp>

//local includes
#include "gtk/Source.h"

//ROOT includes
#include "TGeoManager.h"
#include "TGeoNode.h"
#include "TGeoVolume.h"
#include "TFile.h"
#include "TLorentzVector.h"
#include "TBuffer3D.h"
#include "TDatabasePDG.h"
#include "TParticlePDG.h"
#include "TSystemFile.h"

//c++ includes
#include <regex>

namespace mygl
{
  EvdWindow::EvdWindow(): 
    fViewer(std::unique_ptr<mygl::Camera>(new mygl::PlaneCam(glm::vec3(0., 0., 1000.), glm::vec3(0.0, 1.0, 0.0), 10000., 100.)),
            10., 10., 10.),
    fNextID(0, 0, 0), fServices(), fConfig(new tinyxml2::XMLDocument()), fSource()
  {
    //build_toolbar();

    //fViewer.signal_map().connect(sigc::mem_fun(*this, &EvdWindow::make_scenes)); //Because signal_realize is apparently emitted before this object's 
    //make_scenes();
        
    //show_all_children();                                                                             //children are realize()d
  }

  void EvdWindow::reconfigure(std::unique_ptr<tinyxml2::XMLDocument>&& config)
  {
    fConfig = std::move(config); //Take ownership of config and manage its' lifetime

    if(fConfig)
    {
      //Load global plugins
      auto& geoFactory = plgn::Factory<draw::GeoDrawer>::instance();

      const auto top = fConfig->FirstChildElement();
      const auto drawers = top->FirstChildElement();
      const auto globalConfig = drawers->FirstChildElement("global");
      if(globalConfig)
      {
        tinyxml2::XMLNode* pluginConfig = globalConfig->FirstChildElement();
        while(pluginConfig != nullptr)
        {
          auto drawer = geoFactory.Get(pluginConfig->ToElement());
          if(drawer != nullptr) fGlobalDrawers.push_back(std::move(drawer));
          else std::cerr << "Failed to get global plugin named " << pluginConfig->Value() << "\n";
          pluginConfig = pluginConfig->NextSibling();
        }
      }
      else throw std::runtime_error("Failed to get an element named global from config.xml.\n");

      //Load event plugins
      auto& evtFactory = plgn::Factory<draw::EventDrawer>::instance();
      const auto eventConfig = drawers->FirstChildElement("event"); //TODO: Use a Handle
      if(eventConfig)
      {
        tinyxml2::XMLNode* pluginConfig = eventConfig->FirstChildElement();
        while(pluginConfig != nullptr)
        {
          auto drawer = evtFactory.Get(pluginConfig->ToElement());
          if(drawer != nullptr) fEventDrawers.push_back(std::move(drawer));
          else std::cerr << "Failed to get event plugin named " << pluginConfig->Value() << "\n";
          pluginConfig = pluginConfig->NextSibling();
        }
      }
      else throw std::runtime_error("Failed to get an element named event from config.xml.\n");

      //Load external plugins
      auto& extFactory = plgn::Factory<draw::ExternalDrawer>::instance();
      const auto extConfig = drawers->FirstChildElement("external");
      if(extConfig)
      {
        tinyxml2::XMLNode* pluginConfig = extConfig->FirstChildElement();
        while(pluginConfig != nullptr)
        {
          auto drawer = extFactory.Get(pluginConfig->ToElement());
          if(drawer != nullptr) fExtDrawers.push_back(std::move(drawer));
          else std::cerr << "Failed to get external plugin named " << pluginConfig->Value() << "\n";
          pluginConfig = pluginConfig->NextSibling();
        }
      }
      make_scenes();
    }
  }

  EvdWindow::~EvdWindow() {}

  void EvdWindow::SetSource(std::unique_ptr<src::Source>&& source)
  {
    fSource.reset(source.release()); //= std::move(source);
    //ReadGeo is called when the current file changes, so make sure external drawers are aware of the file change.
    //TODO: I should probably relabel this FileChange() and/or make the negotiation with Source a state machine.
    //TODO: Rethink how Source interacts with ExternalDrawers.  
    if(fSource)
    { 
      for(const auto& draw: fExtDrawers) draw->ConnectTree(fSource->fReader);
      fSource->Next();
      ReadGeo();
      ReadEvent();
    }
  }

  void EvdWindow::ReadGeo()
  {
    fNextID = mygl::VisID();
    
    //Load service information
    const auto serviceConfig = fConfig->FirstChildElement()->FirstChildElement("services"); 
    if(!serviceConfig) std::cerr << "Couldn't find services block.\n";
    const auto geoConfig = serviceConfig->FirstChildElement("geo");
    if(!geoConfig) std::cerr << "Couldn't find geo service.\n";

    fServices.fGeometry.reset(new util::Geometry(geoConfig, fSource->Geo()));
    auto man = fSource->Geo();
    for(const auto& drawPtr: fGlobalDrawers) drawPtr->DrawEvent(*man, fViewer, fNextID);

    std::cout << "Done drawing the geometry.\n";
  }

  void EvdWindow::ReadEvent()
  { 
    std::cout << "Going to next event.\n";
    mygl::VisID id = fNextID; //id gets updated before being passed to the next drawer, but fNextID is only set by the geometry drawer(s)
    const auto& evt = fSource->Event();
    for(const auto& drawPts: fEventDrawers) drawPts->DrawEvent(evt, fViewer, id, fServices);
    std::cout << "Done with event drawers.\n";

    for(const auto& drawer: fExtDrawers) drawer->DrawEvent(evt, fViewer, id, fServices);

    //Last, set current event number for GUI
    //fEvtNum.set_text(std::to_string(fSource->Entry())); //TODO: Does ImGui do this?  

    /*std::vector<LegendView::Row> rows;
    auto db = TDatabasePDG::Instance();
    for(const auto& pdg: *(fServices.fPDGToColor))
    {
      const auto particle = db->GetParticle(pdg.first);
      std::string name;
      if(particle) name = particle->GetName();
      else name = std::to_string(pdg.first);
      Gdk::RGBA color;
      color.set_rgba(pdg.second.r, pdg.second.g, pdg.second.b, 1.0);
      rows.emplace_back(name, color);
    }
    fLegend.reset(new LegendView(*this, std::move(rows)));
    //TODO: Not even close to portable
    fLegend->move(150, 150); //The fact that I specified this in pixels should indicate how frustrated I am...
    fLegend->show();*/
  }
  
  void EvdWindow::make_scenes()
  {
    std::cout << "Calling function EvdWindow::make_scenes()\n";

    //Configure Geometry Scenes
    for(const auto& drawPtr: fGlobalDrawers) drawPtr->RequestScenes(fViewer);    

    //Configure Event Scenes
    for(const auto& drawPtr: fEventDrawers) drawPtr->RequestScenes(fViewer);

    //Configure External Scenes
    for(const auto& extPtr: fExtDrawers) extPtr->RequestScenes(fViewer);

    //ReadGeo();
    //ReadEvent();
  }
  
  void EvdWindow::Print()
  {
    auto window = Gdk::Window::get_default_root_window(); //get_window() //TODO: Why does the main window crash this function?   
    const auto image = Gdk::Pixbuf::create(window, 0, 0, window->get_width(), window->get_height());
    const std::string type = "png"; //TODO: Let the user choose this
    std::stringstream name;
    auto strippedName = fSource->GetFile().substr(0, fSource->GetFile().find_first_of('.')); 
    name << "evd_" << strippedName.substr(strippedName.find_last_of("/")+1, std::string::npos) << "_event_" << fSource->Entry() << "." << type;
    image->save(name.str(), type);
  }

  void EvdWindow::Render(const int width, const int height, const ImGuiIO& ioState)
  {
    //Pop up file selection GUI and call reconfigure()
    if(!fConfig) 
    {
      auto file = fChoose.Render(".xml");
      if(file)
      {
        std::string name(file->GetTitle());
        name += "/";
        name += file->GetName();
        std::unique_ptr<tinyxml2::XMLDocument> doc(new tinyxml2::XMLDocument());
        if(doc->LoadFile(name.c_str()) == tinyxml2::XML_SUCCESS) reconfigure(std::move(doc));
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
      ImGui::SetNextWindowBgAlpha(0.3f); // Transparent background
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

      RenderControlBar();
    }
    fViewer.Render(width, height, ioState);
  }

  void EvdWindow::RenderControlBar()
  {
    //ImGui::Begin("Control Bar");
    ImGui::BeginMainMenuBar();
    {
      if(ImGui::Button("Print"))
      {
        //Print(); //TODO: Implement this in a non-Gtk way
      } 
      ImGui::SameLine();

      int entry = fSource->Entry();
      if(ImGui::InputInt("TTree Entry", &entry))
      {
        goto_event(entry);
      }
      ImGui::SameLine();

      /*int runEvt[2] = {};
      if(ImGui::InputInt2("(RunId, EventId)", runEvt))
      {
        
      }*/ //TODO: For develop branch feature

      if(ImGui::Button("Next")) next_event(); //TODO: Make goto_event() able to detect the end of a file like next_event()
      ImGui::SameLine();
      if(ImGui::Button("Reload")) ReadEvent();
      ImGui::SameLine();
      if(ImGui::Button("File")) fSource.reset(nullptr); //Source reading has already happened, so reset fSource to cause a file chooser 
                                                        //GUI to pop up in next loop.
    }
    ImGui::EndMainMenuBar();
    //ImGui::End();
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

  void EvdWindow::next_event()
  {
    std::cout << "Calling function EvdWindow::next_event()\n";
    if(!fSource->Next()) //If this is the end of the current file
    {
      if(!fSource->NextFile()) //If this is the end of the last file in the Source
      {
        std::cerr << "Reached last event in input files.\n";
      }
      else 
      {
        std::cout << "Reading from file " << fSource->GetFile() << "\n";
        ReadGeo();
        ReadEvent();
      }
    }
    else 
    {
      std::cout << "Reading from file " << fSource->GetFile() << "\n";
      ReadEvent();
    }
  }
}


