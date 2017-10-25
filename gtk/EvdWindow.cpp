//File: EvdWindow.cpp
//Brief: Displays a ROOT geometry with a List Tree view that shows data about each element.
//       Based heavily on https://developer.gnome.org/gtkmm-tutorial/stable/sec-treeview-examples.html.en
//Author: Andrew Olivier

//evd includes
#include "EvdWindow.h"

//gl includes
#include "gl/model/PolyMesh.h"
#include "gl/model/Placed.cpp"
#include "gl/model/Path.h"
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

namespace mygl
{
  //TODO: Something is wrong with starting camera position.  I seem to be placing the camera at what ROOT calls x=-200 by default.  
  //      The camera direction is supposed to be along the z direction as well.  Maybe this is related to the reason why true 
  //      trajectories seem to be reflected?  I like the current starting camera position, but I need to understand it to 
  //      unravel the reason why trajectories seem to be drawn incorrectly.  
  EvdWindow::EvdWindow(const std::string& fileName, const bool darkColors): Gtk::Window(), 
    fViewer(std::shared_ptr<mygl::Camera>(new mygl::PlaneCam(glm::vec3(0., 0., 1000.), glm::vec3(0.0, 1.0, 0.0), 10000., 50.)), 
            darkColors?Gdk::RGBA("(0.f, 0.f, 0.f)"):Gdk::RGBA("(1.f, 1.f, 1.f)"), 10., 10., 10.), 
    fVBox(Gtk::ORIENTATION_VERTICAL), fNavBar(), fPrint("Print"), fNext("Next"), fEvtNumWrap(), fEvtNum(), fFileChoose("File"), 
    fFileName(fileName), fNextID(0, 0, 0), fGeoColor(), fPDGColor(), fPDGToColor()
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
    //Remove the old geometry
    fViewer.GetScenes().find("Geometry")->second.RemoveAll();

    auto man = (TGeoManager*)(fFile->Get("EDepSimGeometry")); //Seems to be a convention in EDepSim
    if(man != nullptr)
    {
      TGeoIdentity id;
      auto top = *(fViewer.GetScenes().find("Geometry")->second.NewTopLevelNode());
      top[fGeoRecord.fName] = man->GetTitle();
      top[fGeoRecord.fMaterial] = "FIXME";
      std::cout << "Generating gemoetry drawing instructions.  This could take a while depending on "
                << "the number of nodes to draw since I currently have to multiply each node's matrix "
                << "by its' parent in c++. This should only happen once per file.  Consider reducing the "
                << "number of nodes in your geometry if this step is prohibitively long since you "
                << "might not even enable them all.  I usually just need to draw the first 5 or so "
                << "layers of the hierarchy.\n";
      AppendNode(man->GetTopNode(), id, top);
      std::cout << "Done generating the geometry.\n";
    } //TODO: else throw exception
  }

  void EvdWindow::ReadEvent()
  { 
    //First, remove the previous event from all scenes except geometry
    for(auto& scenePair: fViewer.GetScenes())
    {
      if(scenePair.first != "Geometry") scenePair.second.RemoveAll();
    }

    //Next, make maps of trackID to particle and parent ID to particle
    std::map<int, std::vector<TG4Trajectory>> parentID;

    for(auto& traj: (*fCurrentEvt)->Trajectories)
    {
      parentID[traj.ParentId].push_back(traj);
    }

    //Then, add particles to list tree and viewer
    auto& primaries = (*fCurrentEvt)->Primaries;
    for(auto& prim: primaries)
    {
      auto row = *(fViewer.GetScenes().find("Event")->second.NewTopLevelNode());
      row[fTrajRecord.fPartName] = prim.Reaction;
      row[fTrajRecord.fEnergy] = -1; //TODO: Use std::regex (maybe) to extract this from prim.Reaction
      row[fTrajRecord.fColor] = Gdk::RGBA("(0., 0., 0.)");
      AppendTrajectories(row, -1, parentID);
    }

    //Last, set current event number for GUI
    fEvtNum.set_text(std::to_string(fReader->GetCurrentEntry()));
  }

  //Passing "mat" BY VALUE in the following recursive function call might cause this application to take up a lot 
  //of memory when opening a file (I estimated O(10MB) in matrix elements for the KLOE geometry with ~120000 nodes), but I think it is 
  //worth the tradeoff for speed which is currently holding the application back at startup.  If I get worried about 
  //the memory these function calls take, I could take an approac like LArSoft's AuxDetGeo constructor that passes around 
  //a list of ancestor node pointers and multiplies all ancestors' matrices for each node.
  Gtk::TreeModel::Row EvdWindow::AppendNode(TGeoNode* node, TGeoMatrix& mat, const Gtk::TreeModel::Row& parent)
  {
    //Get the model matrix for node using it's parent's matrix
    TGeoHMatrix local(*(node->GetMatrix())); //Update TGeoMatrix for this node
    local.MultiplyLeft(&mat);
    double matPtr[16] = {};
    local.GetHomogenousMatrix(matPtr);

    //TODO: Just take the row to add rather than the parent as an argument to AddDrawable().
    //TODO: My trajectories seem to be drawn reflected in x!  
    auto row = fViewer.AddDrawable<Placed<mygl::PolyMesh>>("Geometry", fNextID++, parent, false, glm::make_mat4(matPtr),
                                                           node->GetVolume(), glm::vec4((glm::vec3)fGeoColor, 0.2));
    row[fGeoRecord.fName] = node->GetName();
    row[fGeoRecord.fMaterial] = node->GetVolume()->GetMaterial()->GetName();
    ++fGeoColor;
    //mat = mat*(*(node->GetMatrix())); //Update TGeoMatrix for this node.  This seems to be what LArSoft does in AuxDetGeo's constructor?
    AppendChildren(row, node, local);
    
    return row;
  }  
  
  void EvdWindow::AppendChildren(const Gtk::TreeModel::Row& parent, TGeoNode* parentNode, TGeoMatrix& mat)
  {
    auto children = parentNode->GetNodes();
    //if(children != nullptr && children->GetEntries() != 0) ++fGeoColor; //One color for each level of the geometry hierarchy instead of per node
    for(auto child: *children) AppendNode((TGeoNode*)(child), mat, parent); 
  }

  void EvdWindow::AppendTrajectories(const Gtk::TreeModel::Row& parent, const int trackID, 
                                     std::map<int, std::vector<TG4Trajectory>>& parentToTraj)
  {
    auto trajs = parentToTraj[trackID];
    for(auto& traj: trajs)
    {
      const int pdg = traj.PDGCode;
      //TODO: End Process
      if(fPDGToColor.find(pdg) == fPDGToColor.end()) fPDGToColor[pdg] = fPDGColor++;
      auto color = fPDGToColor[pdg];

      const auto& points = traj.Points;
      std::vector<glm::vec3> vertices;
      for(auto& point: points)
      {
        const auto& pos = point.Position;
        vertices.emplace_back(pos.X(), pos.Y(), pos.Z());
      }

      auto row = fViewer.AddDrawable<mygl::Path>("Event", fNextID, parent, true, vertices, glm::vec4((glm::vec3)color, 1.0)); 
      row[fTrajRecord.fPartName] = traj.Name;
      row[fTrajRecord.fEnergy] = traj.InitialMomentum.E();
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
    fViewer.MakeScene("Event", fTrajRecord, "/home/aolivier/app/evd/src/gl/shaders/colorPerVertex.frag", "/home/aolivier/app/evd/src/gl/shaders/colorPerVertex.vert");
    auto& trajTree = fViewer.GetScenes().find("Event")->second.fTreeView;
    trajTree.append_column("Particle Type", fTrajRecord.fPartName);
    //trajTree.insert_column_with_data_func(-1, "Particle", fPartNameRender, sigc::mem_fun(*this, &EvdWindow::ColToColor));
    trajTree.append_column("Energy", fTrajRecord.fEnergy);
    trajTree.insert_column_with_data_func(-1, "Color", fColorRender, sigc::mem_fun(*this, &EvdWindow::ColToColor));
    //trajTree.append_column("Process", fTrajRecord.fProcess);

    //SetFile(fFileName.c_str());
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
    
    fNavBar.add(fFileChoose);
    fFileChoose.signal_clicked().connect(sigc::mem_fun(*this, &EvdWindow::choose_file));
  }

  void EvdWindow::choose_file()
  {
    Gtk::FileChooserDialog chooser("Choose an edep-sim file to view");
    chooser.set_transient_for(*this);

    chooser.add_button("Open", Gtk::RESPONSE_OK);

    const int result = chooser.run(); 

    if(result == Gtk::RESPONSE_OK)
    {
      fFileName = chooser.get_filename();
      SetFile(fFileName); //TODO: Either remove fFileName or update SetFile() to use fFileName
      std::cout << "Set file to " << fFileName << "\n";
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
}


