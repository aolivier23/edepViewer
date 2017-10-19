//File: EvdWindow.cpp
//Brief: Displays a ROOT geometry with a List Tree view that shows data about each element.
//       Based heavily on https://developer.gnome.org/gtkmm-tutorial/stable/sec-treeview-examples.html.en
//Author: Andrew Olivier

//evd includes
#include "EvdWindow.h"

//gl includes
#include "gl/model/PolyMesh.h"
#include "gl/model/Path.h"
#include "gl/camera/FPSCam.h"

//ROOT includes
#include "TGeoManager.h"
#include "TGeoNode.h"
#include "TGeoVolume.h"
#include "TFile.h"
#include "TLorentzVector.h"

namespace evd
{
  EvdWindow::EvdWindow(const std::string& fileName/*, const Glib::RefPtr<Gtk::Application>& app*/): Gtk::/*Application*/Window(/*app*/), 
    fBox(Gtk::ORIENTATION_HORIZONTAL), 
    fViewer(std::shared_ptr<mygl::Camera>(new mygl::FPSCam(glm::vec3(0., 0., -1000.), glm::vec3(0.0, 0.0, 1.0), 2000., 200.)), 10., 10., 10.),
    fNextID(0, 0, 0), fColor(), fFileName(fileName)
  {
    set_title("Geometry Display Window");
    set_border_width(5);
    set_default_size(1400, 1000);

    add(fBox);
    fBox.set_homogeneous(false);

    fScroll.add(fTree);
    fScroll.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    fBox.pack_start(fViewer);
    fBox.pack_start(fScroll); //, Gtk::PACK_SHRINK);

    fRefTreeModel = Gtk::TreeStore::create(fCols);
    fTree.set_model(fRefTreeModel);

    //TODO: Get text color based on ColorIter working.  This will mean coupling object adding and text adding.  I think I will need object hiding first.
    fNameRender = Gtk::CellRendererText();
    fTree.append_column("Name", fNameRender); //fCols.fNodeName);
    auto col = fTree.get_column(0);
    col->add_attribute(fNameRender.property_text(), fCols.fNodeName);
    col->add_attribute(fNameRender.property_background_rgba(), fCols.fVisID);
    fTree.append_column("Material", fCols.fMaterial);

    fSelection = fTree.get_selection();
    fSelection->signal_changed().connect(sigc::mem_fun(*this, &EvdWindow::DrawSelected));

    fViewer.signal_map().connect(sigc::mem_fun(*this, &EvdWindow::make_scenes)); //Because signal_realize is apparently emitted before this object's 
                                                                                 //children are realize()d

    //SetFile(fileName);

    //fViewer.MakeScene("Geometry");

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
    const auto& trajs = (*fCurrentEvt)->Trajectories;
    for(const auto traj: trajs)
    {
      const auto& points = traj.Points;
      std::vector<glm::vec3> vertices;
      for(auto& point: points)
      {
        const auto& pos = point.Position;
        vertices.emplace_back(pos.X(), pos.Y(), pos.Z());
      }

      fViewer.AddDrawable<mygl::Path>("Trajectories", fNextID, vertices, glm::vec4((glm::vec3)fColor, 1.0)); //TODO: color from PDG code
      //fIDToTraj.emplace(fNextID, traj);
      ++fNextID;
    }
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
    fTree.columns_autosize(); //TODO: Prevent the TreeView from taking over (half of) the world
    //TODO: A TreeView for each Scene and a master TreeView of Scenes (per Viewer?)
    fViewer.MakeScene("Geometry");
    fViewer.MakeScene("Trajectories", "/home/aolivier/app/evd/src/gl/shaders/colorPerVertex.frag", "/home/aolivier/app/evd/src/gl/shaders/colorPerVertex.vert");
    SetFile(fFileName.c_str());
  }
}
