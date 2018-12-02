//File: EvdWindow.h
//Brief: A Gtk::Window for displaying the geometry in a ROOT file.  Has the 
//       ability to draw individual volumes.  Draws both a list tree view of 
//       the geometry hierarchy and a 3D view. 
//Author: Andrew Olivier aolivier@ur.rochester.edu

//Plugin includes
#include "plugins/drawing/geometry/GeoController.cpp"
#include "plugins/drawing/event/EventController.cpp"
#include "plugins/drawing/camera/CameraConfig.cpp"
#include "external/ExternalDrawer.cpp"
#include "plugins/drawing/Services.cpp"
#include "plugins/drawing/ForceDependencyOnLibraries.h"

//local includes
#include "app/FileChoose.h"
#include "app/Source.h"

//gl includes
#include "gl/Viewer.h"
#include "gl/metadata/Column.cpp"

//ROOT includes
#include "TFile.h"
#include "TTreeReader.h"
#include "TGeoMatrix.h"
#include "TDatabasePDG.h"
#include "TSystemDirectory.h"

//yaml-cpp include
#include "yaml-cpp/yaml.h"

//TODO: move EDepSim dependence out of this file
//EDepSim includes
#include "TG4Event.h" 

//c++ includes
#include <future>

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
      void Print(const int width, const int height); //Print the current window to a file

      virtual void make_scenes();

      virtual void reconfigure(std::unique_ptr<YAML::Node>&& config);

      virtual void Render(const int width, const int height, const ImGuiIO& ioState); //Render this window

    protected:
      //Configuration file
      std::unique_ptr<YAML::Node> fConfig;

      //Child Widgets
      mygl::Viewer fViewer;

      //Source of events
      std::unique_ptr<src::Source> fSource;

      void RenderControlBar(const int width, const int height);

    private:
      void ReadGeo();
      void ReadEvent();

      draw::Services fServices;

      //Negotiate with the Source to find the right event 
      void goto_event(const int evt);
      void next_event();

      void goto_id(const int run, const int evt);

      void choose_file(); 
      file::FileChoose fChoose;
      std::unique_ptr<TSystemDirectory> fPwd; //Current ROOT directory used by file selector

      //TODO: Separate event processing details from GUI drawing.  Maybe encapsulate the below objects in EvdController?
      //      Then, ReadGeo() and ReadEvent() would probably become methods of EvdController as well.  
      //plugins
      std::vector<std::unique_ptr<draw::GeoControllerBase>> fGlobalDrawers;
      std::vector<std::unique_ptr<draw::EventControllerBase>> fEventDrawers;
      std::vector<std::unique_ptr<draw::CameraConfig>> fCameraConfigs;
      //std::vector<std::unique_ptr<draw::ExternalDrawer>> fExtDrawers;

      //Keep track of processing of each event. 
      //TODO: Thread spaghetti:
      //      Ultimately, I want to be processing as many events as possible at once.  I should probably place some upper limit on 
      //      the number of events cached.  Retrieving an event from file can also block for a while (think of UChicago wifi), so 
      //      I think I want to retrieve each event to process in a thread that isn't drawing the GUI.  If I can make a copy of each 
      //      event I get, I could send that copy to its own thread and process events in parallel.  
      //
      //      Currently, I'm thinking about creating fSource in its own thread.  The EvdWindow can interact with fSource by telling it 
      //      to stop processing events or asking it for a new event.  Really, whatever runs the Controllers above needs to control fSource.  
      //      When the "master controller" is asked for a new event, it first looks through its cache.  If the next event isn't cached, 
      //      then the "master controller" should tell EvdWindow to wait for the next event to be processed, and the EvdWindow should 
      //      probably poll the "master controller" for status every frame.  
      //
      //      After the "master controller" updates EvdWindow's Viewer, it should try to cache as many events as it can.  Events probably 
      //      have to be loaded in series, so something that is not blocking EvdWindow should start a loop up to the cache size limit that 
      //      takes a copy of an event and dispatches processing of that event to its own thread.

      //For now, I'm just going to process the "next event" in a new thread and wait for it to finish before updating any Scenes.  
      //std::queue<std::future<void>> fEventStatuses; //Status of each thread that is processing an event
      bool fIsWaiting; //Is the application waiting for the current event to be processed?
      std::future<src::Source::metadata> fNextEvent; //When this future is ready, the next event is ready.  
      src::Source::metadata fCurrentEvent; //Source state when current event was first processed
  };
}

#endif //EVD_EVDWINDOW  
