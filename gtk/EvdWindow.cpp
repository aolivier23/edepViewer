//File: EvdWindow.cpp
//Brief: Displays a ROOT geometry with a List Tree view that shows data about each element.
//       Based heavily on https://developer.gnome.org/gtkmm-tutorial/stable/sec-treeview-examples.html.en
//Author: Andrew Olivier

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

//ROOT includes
#include "TGeoManager.h"
#include "TGeoNode.h"
#include "TGeoVolume.h"
#include "TFile.h"
#include "TLorentzVector.h"
#include "TBuffer3D.h"
#include "TDatabasePDG.h"
#include "TParticlePDG.h"

//Tinyxml include
#include <tinyxml2.h>

//c++ includes
#include <regex>

namespace mygl
{
  EvdWindow::EvdWindow(const std::string& fileName, const bool darkColors): Gtk::Window(), 
    fViewer(std::unique_ptr<mygl::Camera>(new mygl::PlaneCam(glm::vec3(0., 0., 1000.), glm::vec3(0.0, 1.0, 0.0), 10000., 100.)), 
            darkColors?Gdk::RGBA("(0.f, 0.f, 0.f)"):Gdk::RGBA("(1.f, 1.f, 1.f)"), 10., 10., 10.), fOverViewer(),  
    fVBox(Gtk::ORIENTATION_VERTICAL), fNavBar(), fPrint("Print"), fNext("Next"), fReload("Reload"), fEvtNumWrap(), fEvtNum(), fFileChoose("File"), fFileLabel(fileName),
    fFileName(fileName), fLegend(nullptr), fNextID(0, 0, 0), fServices()
  {
    set_title("edepsim Display Window");
    set_border_width(5);
    set_default_size(1400, 1000);

    build_toolbar();
    add(fVBox);

    fVBox.pack_start(fNavBar, Gtk::PACK_SHRINK);
    fVBox.pack_end(fViewer);

    fViewer.signal_map().connect(sigc::mem_fun(*this, &EvdWindow::make_scenes)); //Because signal_realize is apparently emitted before this object's 
                                                                                 //children are realize()d


    //TODO: Test of loading an XML document
    tinyxml2::XMLDocument config;
    const auto status = config.LoadFile("config.xml"); //TODO: Gtk::Application shenanigans to read in a file from the command line
    if(status != tinyxml2::XML_SUCCESS) 
    {
      std::cerr << "Got error code " << status << " when loading document config.xml.\n";
      //TODO: Proper error handling by throwing an exception.  I think the Gtk::Application will really handle this anyway.  
    }

    //Load global plugins
    auto& geoFactory = plgn::Factory<draw::GeoDrawer>::instance();

    const auto top = config.FirstChildElement();
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
    else std::cerr << "Failed to get an element named global from config.xml.\n";

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
    else std::cerr << "Failed to get an element named event from config.xml.\n";

    show_all_children();
  }

  EvdWindow::~EvdWindow() {}

  void EvdWindow::SetFile(const std::string& fileName)
  {
    fFile.reset(new TFile(fileName.c_str()));
    fReader.reset(new TTreeReader("EDepSimEvents", fFile.get()));
    fCurrentEvt.reset(new TTreeReaderValue<TG4Event>(*fReader, "Event"));
    fReader->Next(); //TODO: replace with a dedicated event controller
    ReadGeo(); 
    ReadEvent();
  }

  void EvdWindow::ReadGeo()
  {
    fNextID = mygl::VisID();
    
    fGeoManager = (TGeoManager*)(fFile->Get("EDepSimGeometry"));
    auto man = fGeoManager;
    if(man != nullptr)
    {
      for(const auto& drawPtr: fGlobalDrawers) drawPtr->DrawEvent(*man, fViewer, fNextID);

      //Make sure a sensible fiducial node is set
      std::cout << "Done generating the geometry.\n";
    } //TODO: else throw exception
    else std::cerr << "Failed to read geometry manager from file.\n";
  }

  void EvdWindow::ReadEvent()
  { 
    //TODO: Move all per-event drawing code to plugins
    mygl::VisID id = fNextID; //id gets updated before being passed to the next drawer, but fNextID is only set by the geometry drawer(s)
    for(const auto& drawPts: fEventDrawers) drawPts->DrawEvent(*(*fCurrentEvt), *fGeoManager, fViewer, id, fServices);

    //Last, set current event number for GUI
    fEvtNum.set_text(std::to_string(fReader->GetCurrentEntry()));
    auto trajs = fViewer.GetScenes().find("Trajectories");
    if(trajs != fViewer.GetScenes().end()) trajs->second.fTreeView.expand_to_path(Gtk::TreePath("0"));

    //Pop up legend of particle colors used
    std::vector<LegendView::Row> rows;
    auto db = TDatabasePDG::Instance();
    for(const auto& pdg: *(this->fServices.fPDGToColor))
    {
      const std::string name = db->GetParticle(pdg.first)->GetName();
      Gdk::RGBA color;
      color.set_rgba(pdg.second.r, pdg.second.g, pdg.second.b, 1.0);
      rows.emplace_back(name, color);
    }
    fLegend.reset(new LegendView("Particles", *this, std::move(rows)));
    fLegend->set_position(Gtk::WindowPosition::WIN_POS_NONE);
    fLegend->show();
  }
  
  //Function to draw any guides the user requests, like axes or a scale.  Starting with a one meter scale at the 
  //center of the screen broken into mm for now.  
  void EvdWindow::DrawGuides() 
  {
    auto root = *(fViewer.GetScenes().find("Guides")->second.NewTopLevelNode());
    root[fGuideRecord.fName] = "Measurement Objects";
  
    const double gridSize = 1e5;
    //A 1m grid 
    auto row = fViewer.AddDrawable<mygl::Grid>("Guides", fNextID++, root, false, glm::mat4(), gridSize, 1000., gridSize, 1000., 
                                               glm::vec4(0.3f, 0.0f, 0.9f, 0.2f), fLineWidth);
    row[fGuideRecord.fName] = "1m Grid";

    //A 1dm grid 
    row = fViewer.AddDrawable<mygl::Grid>("Guides", fNextID++, root, false, glm::mat4(), gridSize, 100., gridSize, 100.,
                                               glm::vec4(0.3f, 0.0f, 0.9f, 0.2f), fLineWidth);
    row[fGuideRecord.fName] = "1dm Grid";

    //A 1cm grid
    row = fViewer.AddDrawable<mygl::Grid>("Guides", fNextID++, root, false, glm::mat4(), gridSize, 10., gridSize, 10.,
                                               glm::vec4(0.3f, 0.0f, 0.9f, 0.2f), fLineWidth);
    row[fGuideRecord.fName] = "1cm Grid";

    //A 1mm grid
    row = fViewer.AddDrawable<mygl::Grid>("Guides", fNextID++, root, false, glm::mat4(), gridSize, 1., gridSize, 1.,
                                               glm::vec4(0.3f, 0.0f, 0.9f, 0.2f), fLineWidth);
    row[fGuideRecord.fName] = "1mm Grid";
    
    //TODO: Smaller grids, maybe with dashed lines
    //TODO: Drawing the 1mm grid is very slow.  
  }

  void EvdWindow::make_scenes()
  {
    //Configure Geometry Scenes
    for(const auto& drawPtr: fGlobalDrawers) drawPtr->RequestScenes(fViewer);    

    //Configure Event Scenes
    for(const auto& drawPtr: fEventDrawers) drawPtr->RequestScenes(fViewer);

    //Configure guide scene
    auto& guideTree = fViewer.MakeScene("Guides", fGuideRecord, "/home/aolivier/app/evd/src/gl/shaders/userColor.frag", "/home/aolivier/app/evd/src/gl/shaders/HUD.vert", "/home/aolivier/app/evd/src/gl/shaders/wideLine.geom"); 
    guideTree.append_column("Name", fGuideRecord.fName);
    guideTree.expand_to_path(Gtk::TreePath("0"));
    DrawGuides(); //Only do this once ever

    //TODO: Make fFileName a command line option to the application that runs this window.
    choose_file();
  }
  
  void EvdWindow::Print()
  {
    auto window = Gdk::Window::get_default_root_window(); //get_window() //TODO: Why does the main window crash this function?   
    const auto image = Gdk::Pixbuf::create(window, 0, 0, window->get_width(), window->get_height());
    const std::string type = "png"; //TODO: Let the user choose this
    std::stringstream name;
    auto strippedName = fFileName.substr(0, fFileName.find_first_of('.')); 
    name << "evd_" << strippedName.substr(strippedName.find_last_of("/")+1, std::string::npos) << "_event_" << fReader->GetCurrentEntry() << "." << type;
    image->save(name.str(), type);
  }

  void EvdWindow::build_toolbar()
  {
    fNavBar.add(fPrint);
    fPrint.signal_clicked().connect(sigc::mem_fun(*this, &EvdWindow::Print));
    
    fEvtNumWrap.add(fEvtNum);
    fNavBar.add(fEvtNumWrap);
    fEvtNum.signal_activate().connect(sigc::mem_fun(*this, &EvdWindow::goto_event));

    fNavBar.add(fNext);
    fNext.signal_clicked().connect(sigc::mem_fun(*this, &EvdWindow::next_event));

    fNavBar.add(fReload);
    fReload.signal_clicked().connect(sigc::mem_fun(*this, &EvdWindow::ReadEvent));
    
    fNavBar.add(fFileChoose);
    fFileChoose.signal_clicked().connect(sigc::mem_fun(*this, &EvdWindow::choose_file));
    fFileLabelWrap.add(fFileLabel);
    fNavBar.add(fFileLabelWrap);
  }

  void EvdWindow::choose_file()
  {
    Gtk::FileChooserDialog chooser(*this, "Choose an edep-sim file to view");
    //TODO: configure dialog to only show .root files and directories

    chooser.add_button("Open", Gtk::RESPONSE_OK);

    const int result = chooser.run(); 

    if(result == Gtk::RESPONSE_OK)
    {
      fFileName = chooser.get_filename();
      SetFile(fFileName); //TODO: Either remove fFileName or update SetFile() to use fFileName
      std::cout << "Set file to " << fFileName << "\n";
      fFileLabel.set_text(fFileName);
    }
  }

  void EvdWindow::next_event()
  {
    if(fReader->Next()) ReadEvent();
    else fReader->Restart(); //hopefully avoids disaster on next attempt to seek to an event
  }

  void EvdWindow::goto_event()
  {
    const auto newEvt = std::stoi(fEvtNum.get_text());
    if(fReader->SetEntry(newEvt) == TTreeReader::kEntryValid) 
    {
      ReadEvent();
    }
    else std::cerr << "Failed to get event " << newEvt << " from file " << fFileName << "\n";
  }
}


