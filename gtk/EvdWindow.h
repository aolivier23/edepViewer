//File: EvdWindow.h
//Brief: A Gtk::Window for displaying the geometry in a ROOT file.  Has the 
//       ability to draw individual volumes.  Draws both a list tree view of 
//       the geometry hierarchy and a 3D view. 
//Author: Andrew Olivier aolivier@ur.rochester.edu

//Gtkmm includes
#include <gtkmm.h>

//gl includes
#include "gl/Viewer.h"
#include "gl/Scene.h"

//gtk directory includes
#include "ColorIter.cxx"

//ROOT includes
#include "TFile.h"
#include "TTreeReader.h"

//TODO: move EDepSim dependence out of this file
//EDepSim includes
#include "/home/aolivier/app/edep-sim/src/TG4Event.h" //TODO: Make CMake figure out this path

#ifndef EVD_EVDWINDOW
#define EVD_EVDWINDOW

//Forward declaration of ROOT classes
class TGeoNode;

namespace evd
{
  class EvdWindow: public Gtk::Window //Gtk::ApplicationWindow 
  {
    public: 
      EvdWindow(const std::string& fileName); //, const Glib::RefPtr<Gtk::Application>& app);
      virtual ~EvdWindow();

      void SetFile(const std::string& fileName); //Set the file to be processed
      //TODO: Overhaul drawing model to use boolean widgets in TreeView
      void DrawSelected(); //Append children of selected row to list tree
      void Print(); //Print the current window to a file

      virtual void make_scenes();

    protected:
      //TODO: Boolean for whether to draw each object
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
          Gtk::TreeModelColumn<mygl::VisID>         fVisID;
          Gtk::TreeModelColumn<TGeoNode*>     fNode;
      };

      ModelColumns fCols;

      //Child Widgets
      //Gtk::Box fBox;
      Gtk::Paned fBox;
      Gtk::ScrolledWindow fScroll;
      Gtk::TreeView fTree;
      Gtk::CellRendererText fNameRender;
      Glib::RefPtr<Gtk::TreeStore> fRefTreeModel;
      Glib::RefPtr<Gtk::TreeSelection> fSelection;
      mygl::Viewer fViewer;

      //Toolbar for event navigation and printing
      //TODO: event navigation
      Gtk::Box fVBox; //Vertical Box widget for positioning tool bar above drawing area
      Gtk::Toolbar fNavBar;
      Gtk::ToolButton fPrint; 
      Gtk::ToolButton fNext;
      Gtk::ToolItem fEvtNumWrap; 
      Gtk::Entry fEvtNum; 
      Gtk::ToolButton fFileChoose;  

      //Source of data for drawing
      std::string fFileName;
      std::unique_ptr<TFile> fFile;
      std::unique_ptr<TTreeReader> fReader;
      std::unique_ptr<TTreeReaderValue<TG4Event>> fCurrentEvt; //The current event being drawn.

    private:
      Gtk::TreeModel::Row AppendNode(TGeoNode* node, const Gtk::TreeModel::iterator& it);
      void AppendChildren(const Gtk::TreeModel::Row& parent);
      void ReadGeo();
      void ReadEvent();

      mygl::VisID fNextID;
      mygl::ColorIter fColor;

      std::map<int, glm::vec3> fPDGToColor; //TODO: A separate interface from the main window for better organization.  

      void build_toolbar(); //TODO: Write a custom Toolbar class that does this buidling
      void choose_file(); 
      void goto_event();
      void next_event();
  };
}

#endif //EVD_EVDWINDOW  
