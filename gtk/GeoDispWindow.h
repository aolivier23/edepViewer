//File: GeoDispWindow.h
//Brief: A Gtk::Window for displaying the geometry in a ROOT file.  Has the 
//       ability to draw individual volumes.  Draws both a list tree view of 
//       the geometry hierarchy and a 3D view. 
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef EVD_GEODISPWINDOW
#define EVD_GEODISPWINDOW

#include <gtkmm.h>

//Forward declaration of ROOT classes
class TGeoNode;

namespace evd
{
  class GeoDispWindow: public Gtk::Window
  {
    public: 
      GeoDispWindow(const std::string& fileName);
      virtual ~GeoDispWindow() = default;

      void SetFile(const std::string& fileName); //Set the file to be processed
      void AppendSelected(); //Append children of selected row to list tree

    protected:
      class ModelColumns: public Gtk::TreeModel::ColumnRecord
      {
        public:
          ModelColumns()
          {
            add(fNodeName);
            add(fVisID);
            add(fMaterial);
            add(fNode);
          }

          //Visible columns
          Gtk::TreeModelColumn<Glib::ustring> fNodeName;
          Gtk::TreeModelColumn<Glib::ustring> fMaterial; 

          //Hidden columns
          Gtk::TreeModelColumn<int>           fVisID;
          Gtk::TreeModelColumn<TGeoNode*>     fNode;
      };

      ModelColumns fCols;

      //Child Widgets
      Gtk::Box      fHBox;
      Gtk::GLArea   fVisArea;

      Gtk::ScrolledWindow fScroll;
      Gtk::TreeView fTree;
      Glib::RefPtr<Gtk::TreeStore> fRefTreeModel;
      Glib::RefPtr<Gtk::TreeSelection> fSelection;

    private:
      Gtk::TreeModel::Row AppendNode(TGeoNode* node, const Gtk::TreeModel::iterator& it);
      void AppendChildren(const Gtk::TreeModel::Row& parent);

      unsigned int fNextID;
  };
}

#endif //EVD_GEODISPWINDOW  
