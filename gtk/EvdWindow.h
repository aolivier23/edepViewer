//File: EvdWindow.h
//Brief: A Gtk::Window for displaying the geometry in a ROOT file.  Has the 
//       ability to draw individual volumes.  Draws both a list tree view of 
//       the geometry hierarchy and a 3D view. 
//Author: Andrew Olivier aolivier@ur.rochester.edu

//Plugin includes
#include "plugins/drawing/GeoDrawer.cpp"
#include "plugins/drawing/EventDrawer.cpp"
#include "plugins/drawing/Services.cpp"

//Gtkmm includes
#include <gtkmm.h>

//custom GUI includes
#include "gtk/LegendView.h"

//gl includes
#include "gl/Viewer.h"
#include "gl/Scene.h"
#include "gl/ColRecord.cpp"

//ROOT includes
#include "TFile.h"
#include "TTreeReader.h"
#include "TGeoMatrix.h"
#include "TDatabasePDG.h"

//Tinyxml include
#include <tinyxml2.h>

//TODO: move EDepSim dependence out of this file
//EDepSim includes
#include "TG4Event.h" //TODO: Make CMake figure out this path

#ifndef EVD_EVDWINDOW
#define EVD_EVDWINDOW

//Forward declaration of ROOT classes
class TGeoNode;
class TGeoManager;

namespace mygl
{
  class EvdWindow: public Gtk::Window //Gtk::ApplicationWindow 
  {
    public: 
      EvdWindow(const std::string& fileName, const bool darkColors = true); //, const Glib::RefPtr<Gtk::Application>& app);
      virtual ~EvdWindow();

      void SetFile(const std::string& fileName); //Set the file to be processed
      void Print(); //Print the current window to a file

      virtual void make_scenes();

    protected:
      //Configuration file
      std::unique_ptr<tinyxml2::XMLDocument> fConfig;

      //Child Widgets
      mygl::Viewer fViewer;

      //Toolbar for event navigation and printing
      //TODO: event navigation
      Gtk::Box fVBox; //Vertical Box widget for positioning tool bar above drawing area
      Gtk::Toolbar fNavBar;
      Gtk::ToolButton fPrint; 
      Gtk::ToolButton fNext;
      Gtk::ToolButton fReload;
      Gtk::ToolItem fEvtNumWrap; 
      Gtk::Entry fEvtNum; 
      Gtk::ToolButton fFileChoose; 
      Gtk::ToolItem fFileLabelWrap;
      Gtk::Label fFileLabel; //Displays the current file name so that it shows up in printouts 

      std::unique_ptr<LegendView> fLegend;

      //Source of data for drawing
      std::string fFileName;
      std::unique_ptr<TFile> fFile;
      std::unique_ptr<TTreeReader> fReader;
      std::unique_ptr<TTreeReaderValue<TG4Event>> fCurrentEvt; //The current event being drawn.
      std::shared_ptr<TGeoManager> fGeoManager; //The current geometry 

    private:
      void ReadGeo();
      void ReadEvent();

      mygl::VisID fNextID;
      draw::Services fServices;

      void build_toolbar(); //TODO: Write a custom Toolbar class that does this buidling
      void choose_file(); 
      void goto_event();
      void next_event();

      //plugins
      std::vector<std::unique_ptr<draw::GeoDrawer>> fGlobalDrawers;
      std::vector<std::unique_ptr<draw::EventDrawer>> fEventDrawers;

  };
}

#endif //EVD_EVDWINDOW  
