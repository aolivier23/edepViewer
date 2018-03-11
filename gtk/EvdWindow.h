//File: EvdWindow.h
//Brief: A Gtk::Window for displaying the geometry in a ROOT file.  Has the 
//       ability to draw individual volumes.  Draws both a list tree view of 
//       the geometry hierarchy and a 3D view. 
//Author: Andrew Olivier aolivier@ur.rochester.edu

//Plugin includes
#include "plugins/drawing/GeoDrawer.cpp"
#include "plugins/drawing/EventDrawer.cpp"
#include "external/ExternalDrawer.cpp"
#include "plugins/drawing/Services.cpp"

//Gtkmm includes
//#include <gtkmm.h>

//custom GUI includes
//#include "gtk/LegendView.h"

//Source includes
#include "gtk/Source.h"

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
class TGeoManager;
class TChain;

namespace src
{
  class Source;
}

namespace mygl
{
  class EvdWindow
  {
    public: 
      EvdWindow();
      virtual ~EvdWindow();

      void SetSource(std::unique_ptr<src::Source>&& source);
      void Print(); //Print the current window to a file

      virtual void make_scenes();

      virtual void reconfigure(std::unique_ptr<tinyxml2::XMLDocument>&& config);

      virtual void Render(const int width, const int height); //Render this window

    protected:
      //Configuration file
      std::unique_ptr<tinyxml2::XMLDocument> fConfig;

      //Child Widgets
      mygl::Viewer fViewer;

      //Source of events
      std::unique_ptr<src::Source> fSource;

      void RenderControlBar();

      //std::unique_ptr<LegendView> Legend; //TODO: Restore this

    private:
      void ReadGeo();
      void ReadEvent();

      mygl::VisID fNextID;
      draw::Services fServices;

      //Negotiate with the Source to find the right event 
      void goto_event(const int evt);
      void next_event();

      //void build_toolbar(); //TODO: Write a custom Toolbar class that does this buidling
      void choose_file(); 

      //plugins
      std::vector<std::unique_ptr<draw::GeoDrawer>> fGlobalDrawers;
      std::vector<std::unique_ptr<draw::EventDrawer>> fEventDrawers;
      std::vector<std::unique_ptr<draw::ExternalDrawer>> fExtDrawers;
  };
}

#endif //EVD_EVDWINDOW  
