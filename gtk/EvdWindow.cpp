//File: EvdWindow.cpp
//Brief: Displays a ROOT geometry with a List Tree view that shows data about each element.
//       Based heavily on https://developer.gnome.org/gtkmm-tutorial/stable/sec-treeview-examples.html.en
//Author: Andrew Olivier

//evd includes
#include "EvdWindow.h"

//gl includes
#include "gl/model/PolyMesh.h"
#include "gl/model/Path.h"
#include "gl/camera/PlaneCam.h"

//ROOT includes
#include "TGeoManager.h"
#include "TGeoNode.h"
#include "TGeoVolume.h"
#include "TFile.h"
#include "TLorentzVector.h"

namespace evd
{
  EvdWindow::EvdWindow(const std::string& fileName): Gtk::Window(), 
    fBox(Gtk::ORIENTATION_HORIZONTAL),
    fViewer(std::shared_ptr<mygl::Camera>(new mygl::PlaneCam(glm::vec3(0., 0., -500.), glm::vec3(0.0, 1.0, 0.0), 5000., 50.)), 10., 10., 10.),
    fVBox(Gtk::ORIENTATION_VERTICAL), fNavBar(), fPrint("Print"), fNext("Next"), fEvtNumWrap(), fEvtNum(), fFileChoose("File"), 
    fFileName(fileName), fNextID(0, 0, 0), fColor(), fPDGToColor()
  {
    set_title("Geometry Display Window");
    set_border_width(5);
    set_default_size(1400, 1000);

    //fNavBar.add(fPrint);
    build_toolbar();
    add(fVBox);
    fVBox.pack_start(fNavBar, Gtk::PACK_SHRINK);
    fVBox.pack_end(fBox);

    fScroll.add(fTree);
    fScroll.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    fBox.pack1(fViewer);
    fBox.pack2(fScroll);

    fRefTreeModel = Gtk::TreeStore::create(fCols);
    fTree.set_model(fRefTreeModel);

    //TODO: Get text color based on ColorIter working.  This will mean coupling object adding and text adding.  I think I will need object hiding first.
    fNameRender = Gtk::CellRendererText();
    fTree.append_column("Name", fCols.fNodeName); //fNameRender);
    //TODO: Make colors of entry names match colors of Drawables
    /*auto col = fTree.get_column(0);
    col->add_attribute(fNameRender.property_text(), fCols.fNodeName);
    col->add_attribute(fNameRender.property_background_rgba(), fCols.fVisID);*/
    fTree.append_column("Material", fCols.fMaterial);

    fSelection = fTree.get_selection();
    fSelection->signal_changed().connect(sigc::mem_fun(*this, &EvdWindow::DrawSelected));

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
    //TODO: Also remove from list tree -> further support for scene-by-scene list trees

    auto man = (TGeoManager*)(fFile->Get("EDepSimGeometry")); //Seems to be a convention in EDepSim
    std::cout << "Sucessfully got TGeoManager\n";
    if(man != nullptr)
    {
      std::cout << "Top node is named " << man->GetTopNode()->GetName() << "\n";
      AppendNode(man->GetTopNode(), fRefTreeModel->append());
    } //TODO: else throw exception
  }

  void EvdWindow::ReadEvent()
  {
    //std::map<mygl::VisID, TG4Trajectory*> fIDToTraj;
    //First, remove the previous event from all scenes except geometry
    for(auto& scenePair: fViewer.GetScenes())
    {
      if(scenePair.first != "Geometry") scenePair.second.RemoveAll();
    }

    const auto& trajs = (*fCurrentEvt)->Trajectories;
    for(const auto traj: trajs)
    {
      const int pdg = traj.PDGCode;
      if(fPDGToColor.find(pdg) == fPDGToColor.end()) fPDGToColor[pdg] = fColor++;
      auto color = fPDGToColor[pdg];

      const auto& points = traj.Points;
      std::vector<glm::vec3> vertices;
      for(auto& point: points)
      {
        const auto& pos = point.Position;
        vertices.emplace_back(pos.X(), pos.Y(), pos.Z());
      }

      fViewer.AddDrawable<mygl::Path>("Event", fNextID, vertices, glm::vec4((glm::vec3)color, 1.0)); //TODO: color from PDG code
      //fIDToTraj.emplace(fNextID, traj);
      ++fNextID;
    }
    fEvtNum.set_text(std::to_string(fReader->GetCurrentEntry()));
  }

  Gtk::TreeModel::Row EvdWindow::AppendNode(TGeoNode* node, const Gtk::TreeModel::iterator& it)
  {
    auto row = *it;
    row[fCols.fNodeName] = node->GetName();
    row[fCols.fMaterial] = node->GetVolume()->GetMaterial()->GetName();
    row[fCols.fVisID] = fNextID++;
    row[fCols.fNode] = node;
    AppendChildren(row);
    
    return row;
  }  
  
  void EvdWindow::AppendChildren(const Gtk::TreeModel::Row& parent)
  {
    TGeoNode* parentNode = parent[fCols.fNode];
    auto children = parentNode->GetNodes();
    for(auto child: *children) AppendNode((TGeoNode*)(child), fRefTreeModel->append(parent.children())); 
  }

  void EvdWindow::DrawSelected()
  {
    std::cout << "Called EvdWindow::DrawSelected()\n";
    TGeoNode* node = (*(fSelection->get_selected()))[fCols.fNode];
    std::cout << "Adding drawable with ID " << fNextID << "\n";
    fViewer.AddDrawable<mygl::PolyMesh>("Geometry", fNextID, node->GetVolume(), glm::vec4((glm::vec3)fColor, 0.2)); 
    ++fNextID;
    std::cout << "fNextID is now " << fNextID << "\n";
    ++fColor;
  }

  void EvdWindow::make_scenes()
  {
    fBox.set_position(get_width()*0.8);
    //TODO: A TreeView for each Scene and a master TreeView of Scenes (per Viewer?)
    fViewer.MakeScene("Geometry");
    fViewer.MakeScene("Event", "/home/aolivier/app/evd/src/gl/shaders/colorPerVertex.frag", "/home/aolivier/app/evd/src/gl/shaders/colorPerVertex.vert");
    SetFile(fFileName.c_str());
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
}


