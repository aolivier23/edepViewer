//File: GeoDrawWindow.cpp
//Brief: Displays a ROOT geometry with a List Tree view that shows data about each element.
//       Based heavily on https://developer.gnome.org/gtkmm-tutorial/stable/sec-treeview-examples.html.en
//Author: Andrew Olivier

//evd includes
#include "GeoDrawWindow.h"

//ROOT includes
#include "TGeoManager.h"
#include "TGeoNode.h"
#include "TGeoVolume.h"

namespace evd
{
  GeoDrawWindow::GeoDrawWindow(const std::string& fileName): Gtk::ApplicationWindow(), fBox(Gtk::ORIENTATION_HORIZONTAL), 
    fArea(600, 1000, std::shared_ptr<mygl::Camera>(new mygl::FPSCam(glm::vec3(0., 0., -1000.)))), fNextID(0), fColor()
  {
    set_title("Geometry Display Window");
    set_border_width(5);
    set_default_size(1200, 1000);

    add(fBox);

    fScroll.add(fTree);
    fScroll.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    fBox.pack_start(fArea);
    fBox.pack_start(fScroll);

    fRefTreeModel = Gtk::TreeStore::create(fCols);
    fTree.set_model(fRefTreeModel);

    fTree.append_column("Name", fCols.fNodeName);
    fTree.append_column("Material", fCols.fMaterial);

    fSelection = fTree.get_selection();
    fSelection->signal_changed().connect(sigc::mem_fun(*this, &GeoDrawWindow::DrawSelected));

    SetFile(fileName);

    show_all_children();
  }

  void GeoDrawWindow::SetFile(const std::string& fileName)
  {
    auto man = TGeoManager::Import(fileName.c_str());
    if(man != nullptr)
    {
      AppendNode(man->GetTopNode(), fRefTreeModel->append());
    }
  }

  Gtk::TreeModel::Row GeoDrawWindow::AppendNode(TGeoNode* node, const Gtk::TreeModel::iterator& it)
  {
    auto row = *it;
    row[fCols.fNodeName] = node->GetName();
    row[fCols.fMaterial] = node->GetVolume()->GetMaterial()->GetName();
    row[fCols.fVisID] = fNextID++;
    row[fCols.fNode] = node;
    AppendChildren(row);
    
    return row;
  }  
  
  void GeoDrawWindow::AppendChildren(const Gtk::TreeModel::Row& parent)
  {
    TGeoNode* parentNode = parent[fCols.fNode];
    auto children = parentNode->GetNodes();
    for(auto child: *children) AppendNode((TGeoNode*)(child), fRefTreeModel->append(parent.children())); 
  }

  void GeoDrawWindow::DrawSelected()
  {
    std::cout << "Called GeoDrawWindow::DrawSelected()\n";
    TGeoNode* node = (*(fSelection->get_selected()))[fCols.fNode];
    if(fArea.fDrawer == nullptr)
    {
      std::cerr << "Tried to access fDrawer in mygl::GeoDrawWindow::DrawSelected(), but it was nullptr.  So, not doing anything.\n";
      return;
    }
    fArea.make_current(); //!!! TODO: This implies that Gtk can use other Gdk::GLContexts in the meantime.  
                          //          I should rethink my drawing model to make sure Gtk::GLArea::make_current() is 
                          //          called before I start binding VAOs and friends in DRAWER classes.   
    fArea.fDrawer->AddGeo(node->GetVolume(), glm::vec4((glm::vec3)fColor, 0.2));
    ++fColor;
  }
}
