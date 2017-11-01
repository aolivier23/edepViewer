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
#include "gl/camera/PlaneCam.h"

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

//c++ includes
#include <regex>

namespace mygl
{
  EvdWindow::EvdWindow(const std::string& fileName, const bool darkColors): Gtk::Window(), 
    fViewer(std::unique_ptr<mygl::Camera>(new mygl::PlaneCam(glm::vec3(0., 0., 1000.), glm::vec3(0.0, 1.0, 0.0), 10000., 100.)), 
            darkColors?Gdk::RGBA("(0.f, 0.f, 0.f)"):Gdk::RGBA("(1.f, 1.f, 1.f)"), 10., 10., 10.), 
    fVBox(Gtk::ORIENTATION_VERTICAL), fNavBar(), fPrint("Print"), fNext("Next"), fReload("Reload"), fEvtNumWrap(), fEvtNum(), fFileChoose("File"), fFileLabel(fileName),
    fFileName(fileName), fNextID(0, 0, 0), fGeoColor(new mygl::ColorIter()), fPDGColor(), fPDGToColor(), fPdgDB(), fMaxGeoDepth(7), 
    //TODO: Make fMaxGeoDepth a user parameter
    fPalette(1.0, 50.)
  {
    set_title("Geometry Display Window");
    set_border_width(5);
    set_default_size(1400, 1000);

    build_toolbar();
    add(fVBox);
    fVBox.pack_start(fNavBar, Gtk::PACK_SHRINK);
    fVBox.pack_end(fViewer);

    fViewer.signal_map().connect(sigc::mem_fun(*this, &EvdWindow::make_scenes)); //Because signal_realize is apparently emitted before this object's 
                                                                                 //children are realize()d


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
    fGeoColor.reset(new mygl::ColorIter());
    //Remove the old geometry
    fViewer.GetScenes().find("Geometry")->second.RemoveAll();
    //TODO: Reset first VisID to (0, 0, 0)?
    //TODO: Reset geometry color iterator

    //auto man = (TGeoManager*)(fFile->Get("EDepSimGeometry")); //Seems to be a convention in EDepSim
    fGeoManager = (TGeoManager*)(fFile->Get("EDepSimGeometry"));
    auto man = fGeoManager;
    if(man != nullptr)
    {
      auto id = new TGeoIdentity(); //This should be a memory leak in any reasonable framework, but TGeoIdentity registers itself
                                    //with TGeoManager so that TGeoManager will try to delete it in ~TGeoManager().  Furthermore, 
                                    //I have yet to find a way to unregister a TGeoMatrix.  So, it appears that there is no such 
                                    //thing as a temporary TGeoIdentity.  Good job ROOT... :(
      auto top = *(fViewer.GetScenes().find("Geometry")->second.NewTopLevelNode());
      top[fGeoRecord.fName] = man->GetTitle();
      top[fGeoRecord.fMaterial] = "FIXME";
      std::cout << "Generating gemoetry drawing instructions.  This could take a while depending on "
                << "the number of nodes to draw since I currently have to multiply each node's matrix "
                << "by its' parent in c++. This should only happen once per file.  Consider reducing the "
                << "number of nodes in your geometry if this step is prohibitively long since you "
                << "might not even enable them all.  I usually just need to draw the first 5 or so "
                << "layers of the hierarchy.\n";
      AppendNode(man->GetTopNode(), *id, top, 0);
      fAfterLastGeo = fNextID;

      //Make sure a sensible fiducial node is set
      fFiducialName.set_text(man->GetTopNode()->GetName());
      set_fiducial();
      std::cout << "Done generating the geometry.\n";
    } //TODO: else throw exception
    else std::cerr << "Failed to read geometry manager from file.\n";
  }

  void EvdWindow::ReadEvent()
  { 
    //First, remove the previous event from all scenes except geometry
    for(auto& scenePair: fViewer.GetScenes())
    {
      if(scenePair.first != "Geometry" && scenePair.first != "Guides") scenePair.second.RemoveAll();
    }
    fNextID = fAfterLastGeo;

    //Next, make maps of trackID to particle and parent ID to particle
    std::map<int, std::vector<TG4Trajectory>> parentID;

    for(auto& traj: (*fCurrentEvt)->Trajectories)
    {
      parentID[traj.ParentId].push_back(traj);
    }

    //Then, add particles to list tree and viewer
    std::regex genieToEvd(R"(nu:(-?[[:digit:]]+);tgt:([[:digit:]]+);N:([[:digit:]]+)(.*);proc:Weak\[([[:alpha:]]+)\],([[:alpha:]]+);(.*))");
    auto& primaries = (*fCurrentEvt)->Primaries;
    for(auto& prim: primaries)
    {
      auto row = *(fViewer.GetScenes().find("Trajectories")->second.NewTopLevelNode());
      
      //Turn GENIE's interaction string into something easier to read
      std::smatch match;
      if(!std::regex_match(prim.Reaction, match, genieToEvd)) std::cerr << "Got interaction string from GENIE that does not match what I expect:\n"
                                                                        << prim.Reaction << "\n";

      const std::string nu = fPdgDB.GetParticle(std::stoi(match[1].str()))->GetName();
      //const std::string nucleus = fPdgDB.GetParticle(match[2].str().c_str())->GetName(); //TODO: TDatabasPDG can't read PDG codes for nuclei
      const std::string nucleon = fPdgDB.GetParticle(std::stoi(match[3].str()))->GetName(); 
      row[fTrajRecord.fPartName] = nu+" "+match[5].str()+" "+match[6].str();//+" on "/*+nucleus+" "*/+nucleon;
      row[fTrajRecord.fEnergy] = -1; //TODO: Use std::regex (maybe) to extract this from prim.Reaction
      row[fTrajRecord.fColor] = Gdk::RGBA("(0., 0., 0.)");
      AppendTrajectories(row, -1, parentID);
    }

    //Last, set current event number for GUI
    fEvtNum.set_text(std::to_string(fReader->GetCurrentEntry()));
    fViewer.GetScenes().find("Trajectories")->second.fTreeView.expand_to_path(Gtk::TreePath("0")); //TODO: Move this to the Viewer?

    //Draw true energy deposits color-coded by energy
    auto edepToDet = (*fCurrentEvt)->SegmentDetectors; //A map from sensitive volume to energy deposition

    for(auto& det: edepToDet)
    {
      auto detName = det.first;
      auto detRow = *(fViewer.GetScenes().find("EDep")->second.NewTopLevelNode());
      auto edeps = det.second;
      detRow[fEDepRecord.fPrimName] = detName;
      //TODO: Map sensitive volume name to detector name and use same VisID?  This would seem to require infrastructure that 
      //      I don't yet have both in terms of Scenes communicating when an object is toggled (not too hard) and ROOT not 
      //      knowing about sensitive detector auxiliary tags (potentially very hard).  
      //TODO: Energy depositions children of sensitive detector?  This doesn't seem so useful at the moment, and sensitive detectors 
      //      do not have the "properties" I plan to include in the energy deposition tree. I will just cut out energy depositions 
      //      in volumes by setting the frustrum box from the camera API (hopefully).   
      double sumE = 0., sumScintE = 0., minT = 1e10;
      for(auto& edep: edeps)
      {
        const auto start = edep.Start;
        glm::vec3 firstPos(start.X(), start.Y(), start.Z());
        const auto stop = edep.Stop;
        glm::vec3 lastPos(stop.X(), stop.Y(), stop.Z());
        const auto energy = edep.EnergyDeposit;

        auto row = fViewer.AddDrawable<mygl::Path>("EDep", fNextID, detRow, true, glm::mat4(), std::vector<glm::vec3>{firstPos, lastPos}, glm::vec4(fPalette(energy), 1.0));
        row[fEDepRecord.fScintE]  = edep.SecondaryDeposit;
        row[fEDepRecord.fEnergy]  = energy;
        row[fEDepRecord.fT0]      = start.T();
        row[fEDepRecord.fPrimName] = (*fCurrentEvt)->Trajectories[edep.PrimaryId].Name; //TODO: energy depositions children of contributing tracks?
        ++fNextID;

        sumE += energy;
        sumScintE += edep.SecondaryDeposit;
        if(start.T() < minT) minT = start.T();
      }

      //Fill values for this sensitive detector with cumulative energy from its' hits and time of earliest hit
      detRow[fEDepRecord.fEnergy] = sumE;
      detRow[fEDepRecord.fScintE] = sumScintE;
      detRow[fEDepRecord.fT0]     = minT;
    }
  }
  
  //Function to draw any guides the user requests, like axes or a scale.  Starting with a one meter scale at the 
  //center of the screen broken into mm for now.  
  void EvdWindow::DrawGuides() //TODO: This should become a Viewer function
  {
    auto root = *(fViewer.GetScenes().find("Guides")->second.NewTopLevelNode());
    root[fGuideRecord.fName] = "Measurement Objects";
  
    const double gridSize = 1e5;
    //A 1m grid 
    auto row = fViewer.AddDrawable<mygl::Grid>("Guides", fNextID++, root, false, glm::mat4(), gridSize, 1000., gridSize, 1000., 
                                               glm::vec4(0.3f, 0.0f, 0.9f, 0.2f));
    row[fGuideRecord.fName] = "1m Grid";

    //A 1dm grid 
    row = fViewer.AddDrawable<mygl::Grid>("Guides", fNextID++, root, false, glm::mat4(), gridSize, 100., gridSize, 100.,
                                               glm::vec4(0.3f, 0.0f, 0.9f, 0.2f));
    row[fGuideRecord.fName] = "1dm Grid";

    //A 1cm grid
    row = fViewer.AddDrawable<mygl::Grid>("Guides", fNextID++, root, false, glm::mat4(), gridSize, 10., gridSize, 10.,
                                               glm::vec4(0.3f, 0.0f, 0.9f, 0.2f));
    row[fGuideRecord.fName] = "1cm Grid";

    //A 1mm grid
    row = fViewer.AddDrawable<mygl::Grid>("Guides", fNextID++, root, false, glm::mat4(), gridSize, 1., gridSize, 1.,
                                               glm::vec4(0.3f, 0.0f, 0.9f, 0.2f));
    row[fGuideRecord.fName] = "1mm Grid";
    
    //TODO: Smaller grids, maybe with dashed lines
  }

  Gtk::TreeModel::Row EvdWindow::AppendNode(TGeoNode* node, TGeoMatrix& mat, const Gtk::TreeModel::Row& parent, size_t depth)
  {
    //Get the model matrix for node using it's parent's matrix
    TGeoHMatrix local(*(node->GetMatrix())); //Update TGeoMatrix for this node
    local.MultiplyLeft(&mat);
    double matPtr[16] = {};
    local.GetHomogenousMatrix(matPtr);

    //TODO: Just take the row to add rather than the parent as an argument to AddDrawable().
    //TODO: Adding the line immediately below this causes weird GUI behavior.  The problem does not seem to be in Scene or Polymesh::Draw().  
    auto row = fViewer.AddDrawable<mygl::PolyMesh>("Geometry", fNextID++, parent, false, glm::make_mat4(matPtr),
                                                           node->GetVolume(), glm::vec4((glm::vec3)(*fGeoColor), 0.2)); 
                                                                                               
    row[fGeoRecord.fName] = node->GetName();
    row[fGeoRecord.fMaterial] = node->GetVolume()->GetMaterial()->GetName();
    ++(*fGeoColor);
    AppendChildren(row, node, local, depth);
    
    return row;
  }  
  
  void EvdWindow::AppendChildren(const Gtk::TreeModel::Row& parent, TGeoNode* parentNode, TGeoMatrix& mat, size_t depth)
  {
    auto children = parentNode->GetNodes();
    //if(children != nullptr && children->GetEntries() != 0) ++fGeoColor; //One color for each level of the geometry hierarchy instead of per node
    if(depth == fMaxGeoDepth) return;
    for(auto child: *children) AppendNode((TGeoNode*)(child), mat, parent, depth+1); 
  }

  void EvdWindow::AppendTrajectories(const Gtk::TreeModel::Row& parent, const int trackID, 
                                     std::map<int, std::vector<TG4Trajectory>>& parentToTraj)
  {
    auto trajs = parentToTraj[trackID];
    for(auto& traj: trajs)
    {
      const int pdg = traj.PDGCode;
      //TODO: End Process
      //TODO: Legend
      if(fPDGToColor.find(pdg) == fPDGToColor.end()) fPDGToColor[pdg] = fPDGColor++;
      auto color = fPDGToColor[pdg];

      const auto& points = traj.Points;
      std::vector<glm::vec3> vertices;
      for(auto& point: points)
      {
        const auto& pos = point.Position;

        //Require that point is inside fiducial volume
        double master[] = {pos.X(), pos.Y(), pos.Z()};
        double local[3] = {}; 
        fFiducialMatrix->MasterToLocal(master, local);
        if(fFiducialNode->GetVolume()->Contains(local)) vertices.emplace_back(pos.X(), pos.Y(), pos.Z());
      }

      auto row = fViewer.AddDrawable<mygl::Path>("Trajectories", fNextID, parent, true, glm::mat4(), vertices, glm::vec4((glm::vec3)color, 1.0)); 
      row[fTrajRecord.fPartName] = traj.Name;
      auto p = traj.InitialMomentum;
      const double invariantMass = std::sqrt((p.Mag2()>0)?p.Mag2():0); //Make sure the invariant mass is > 0 as it should be.  It might be negative for 
                                                                       //photons because of floating point precision behavior.  Never trust a computer to 
                                                                       //calculate 0...
      row[fTrajRecord.fEnergy] = p.E()-invariantMass; //Kinetic energy
      Gdk::RGBA gdkColor;
      gdkColor.set_rgba(color.r, color.g, color.b, 1.0);
      row[fTrajRecord.fColor] = gdkColor;
      ++fNextID;
      AppendTrajectories(row, traj.TrackId, parentToTraj);
    }
  }

  void EvdWindow::make_scenes()
  {
    //Configure Geometry Scene
    fViewer.MakeScene("Geometry", fGeoRecord);
    auto& geoTree = fViewer.GetScenes().find("Geometry")->second.fTreeView;
    geoTree.append_column("Volume Name", fGeoRecord.fName);
    geoTree.append_column("Material", fGeoRecord.fMaterial);
 
    //Configure Event Scene
    fViewer.MakeScene("Trajectories", fTrajRecord, "/home/aolivier/app/evd/src/gl/shaders/colorPerVertex.frag", "/home/aolivier/app/evd/src/gl/shaders/colorPerVertex.vert");
    auto& trajTree = fViewer.GetScenes().find("Trajectories")->second.fTreeView;
    trajTree.append_column("Particle Type", fTrajRecord.fPartName);
    //trajTree.insert_column_with_data_func(-1, "Particle", fPartNameRender, sigc::mem_fun(*this, &EvdWindow::ColToColor));
    trajTree.append_column("KE [MeV]", fTrajRecord.fEnergy);
    trajTree.insert_column_with_data_func(-1, "Color", fColorRender, sigc::mem_fun(*this, &EvdWindow::ColToColor));
    //trajTree.append_column("Process", fTrajRecord.fProcess);

    //Configure guide scene
    fViewer.MakeScene("Guides", fGuideRecord, "/home/aolivier/app/evd/src/gl/shaders/userColor.frag", "/home/aolivier/app/evd/src/gl/shaders/HUD.vert"); 
    auto& guideTree = fViewer.GetScenes().find("Guides")->second.fTreeView;
    guideTree.append_column("Name", fGuideRecord.fName);

    DrawGuides(); //Only do this once ever

    fViewer.MakeScene("EDep", fEDepRecord, "/home/aolivier/app/evd/src/gl/shaders/colorPerVertex.frag", "/home/aolivier/app/evd/src/gl/shaders/colorPerVertex.vert");
    auto& edepTree = fViewer.GetScenes().find("EDep")->second.fTreeView;
    edepTree.append_column("Main Contributor", fEDepRecord.fPrimName);
    edepTree.append_column("Energy [MeV]", fEDepRecord.fEnergy);
    edepTree.append_column("Scintillation Energy [MeV]", fEDepRecord.fScintE);
    edepTree.append_column("Start Time [ns?]", fEDepRecord.fT0);

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

    fFiducialLabelWrap.add(fFiducialLabel);
    fNavBar.add(fFiducialLabelWrap);
    fFiducialWrap.add(fFiducialName);
    fNavBar.add(fFiducialWrap);
    fFiducialName.signal_activate().connect(sigc::mem_fun(*this, &EvdWindow::set_fiducial));
  }

  void EvdWindow::choose_file()
  {
    Gtk::FileChooserDialog chooser(*this, "Choose an edep-sim file to view");

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

  void EvdWindow::ColToColor(Gtk::CellRenderer* render, const Gtk::TreeModel::iterator& it)
  {
    Gdk::RGBA color = (*it)[fTrajRecord.fColor];
    auto textRender = ((Gtk::CellRendererText*)render);
    textRender->property_background_rgba().set_value(color);
  }

  void EvdWindow::set_fiducial()
  {
    const std::string nodeName = fFiducialName.get_text();

    auto id = new TGeoIdentity();
    if(find_node(nodeName, fGeoManager->GetTopNode(), *id))
    {
      std::cout << "Set fiducial node to " << fFiducialNode->GetName() << "\n";
      ReadEvent();
    }
    else
    {
      fFiducialName.set_text(fFiducialNode->GetName());
      std::cerr << "Failed to find node with name " << nodeName << " in file " << fFile->GetName() << "\n";
    }
  }

  bool EvdWindow::find_node(const std::string& name, TGeoNode* parent, TGeoMatrix& mat)
  {
    TGeoHMatrix local(mat);
    local.Multiply(parent->GetMatrix());
    if(std::string(parent->GetName()) == name) 
    {
      fFiducialNode = parent;
      fFiducialMatrix = new TGeoHMatrix(local);
      return true;
    }

    auto children = parent->GetNodes();
    for(auto child: *children)
    {
      auto node = (TGeoNode*)child;
      if(find_node(name, node, local)) return true;
    }
    return false;
  }
}


