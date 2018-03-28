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

//local includes
#include "app/FileChoose.h"

//gl includes
#include "gl/Viewer.h"
#include "gl/Scene.h"
#include "gl/ColRecord.cpp"

//ROOT includes
#include "TFile.h"
#include "TTreeReader.h"
#include "TGeoMatrix.h"
#include "TDatabasePDG.h"
#include "TSystemDirectory.h"

//Tinyxml include
#include <tinyxml2.h>

//TODO: move EDepSim dependence out of this file
//EDepSim includes
#include "EDepSim/TG4Event.h" 

#ifndef EVD_EVDWINDOW
#define EVD_EVDWINDOW

//Forward declaration of ROOT classes
class TGeoManager;
class TChain;

namespace src
{
  class Source;
}

namespace file
{
  class FileChoose;
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

      virtual void Render(const int width, const int height, const ImGuiIO& ioState); //Render this window

    protected:
      //Configuration file
      std::unique_ptr<tinyxml2::XMLDocument> fConfig;

      //Child Widgets
      mygl::Viewer fViewer;

      //Source of events
      std::unique_ptr<src::Source> fSource;

      void RenderControlBar();

    private:
      void ReadGeo();
      void ReadEvent();

      mygl::VisID fNextID;
      draw::Services fServices;

      //Negotiate with the Source to find the right event 
      void goto_event(const int evt);
      void next_event();

      void goto_id(const int run, const int evt);

      void choose_file(); 
      file::FileChoose fChoose;
      std::unique_ptr<TSystemDirectory> fPwd; //Current ROOT directory used by file selector

      //plugins
      std::vector<std::unique_ptr<draw::GeoDrawer>> fGlobalDrawers;
      std::vector<std::unique_ptr<draw::EventDrawer>> fEventDrawers;
      std::vector<std::unique_ptr<draw::ExternalDrawer>> fExtDrawers;
  };
}

#endif //EVD_EVDWINDOW  
