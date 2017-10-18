//File: GeoDispWindow.cpp
//Brief: Displays a ROOT geometry with a List Tree view that shows data about each element.
//       Based heavily on https://developer.gnome.org/gtkmm-tutorial/stable/sec-treeview-examples.html.en
//Author: Andrew Olivier

//evd includes
#include "GeoDispWindow.h"

//ROOT includes
#include "TGeoManager.h"
#include "TGeoNode.h"
#include "TGeoVolume.h"

namespace evd
{
  GeoDispWindow::GeoDispWindow(const std::string& fileName): fHBox(Gtk::ORIENTATION_HORIZONTAL), fVisArea(), fNextID(0)
  {
    set_title("Geometry Display Window");
    set_border_width(5);
    set_default_size(400, 200);

    add(fHBox);

    fScroll.add(fTree);
    fScroll.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    fHBox.pack_start(fVisArea);
    fHBox.pack_start(fScroll);

    fRefTreeModel = Gtk::TreeStore::create(fCols);
    fTree.set_model(fRefTreeModel);

    fTree.append_column("Name", fCols.fNodeName);
    fTree.append_column("Material", fCols.fMaterial);

    fSelection = fTree.get_selection();
    fSelection->signal_changed().connect(sigc::mem_fun(*this, &GeoDispWindow::AppendSelected));

    SetFile(fileName);

    show_all_children();
  }

  void GeoDispWindow::SetFile(const std::string& fileName)
  {
    auto man = TGeoManager::Import(fileName.c_str());
    if(man != nullptr)
    {
      AppendNode(man->GetTopNode(), fRefTreeModel->append());
    }
  }

  Gtk::TreeModel::Row GeoDispWindow::AppendNode(TGeoNode* node, const Gtk::TreeModel::iterator& it)
  {
    auto row = *it;
    row[fCols.fNodeName] = node->GetName();
    row[fCols.fMaterial] = node->GetVolume()->GetMaterial()->GetName();
    row[fCols.fVisID] = fNextID++;
    row[fCols.fNode] = node;
    
    return row;
  }  
  
  void GeoDispWindow::AppendChildren(const Gtk::TreeModel::Row& parent)
  {
    TGeoNode* parentNode = parent[fCols.fNode];
    auto children = parentNode->GetNodes();
    for(auto child: *children) AppendNode((TGeoNode*)(child), fRefTreeModel->append(parent.children())); 
  }

  void GeoDispWindow::AppendSelected()
  {
    auto selected = *(fSelection->get_selected());
    if(selected.children().size() == 0) AppendChildren(*(fSelection->get_selected()));
  }
}
