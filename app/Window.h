//File: Window.h
//Brief: A Window for drawing 3D event viewers along with controls.  Use evd::Controller 
//       to run an evd::Window.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//Plugin includes
#include "plugins/drawing/geometry/GeoController.cpp"
#include "plugins/drawing/event/EventController.cpp"
#include "plugins/drawing/camera/CameraConfig.cpp"
#include "external/ExternalDrawer.cpp"
#include "plugins/drawing/Services.cpp"
#include "plugins/drawing/ForceDependencyOnLibraries.h"

//local includes
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

#ifndef EVD_WINDOW
#define EVD_WINDOW

//Forward declaration of ROOT classes
class TGeoManager;
class TChain;

namespace src
{
  class Source;
}

namespace evd
{
  class Controller;

  class Window
  {
    public: 
      Window(std::unique_ptr<YAML::Node>&& config, std::unique_ptr<src::Source>&& source);
      virtual ~Window();

      friend class Controller;

      //State interacts with Window through this interface
      //Control event processing
      void ProcessEvent(const bool forceGeo);  //Process the next event in the current Source
      void ProcessEvent(const int run, const int event); //Process a specific event from the current Source
      void LoadNextEvent(); //Load the first event in fEventCache as the event the user is viewing
      void ClearCache(); //Empty fEventCache.  Useful in preparation for a non-sequential event access
      void SetSource(std::unique_ptr<src::Source>&& source); //Set the Source from which future events will be read

      //Functions that can be called at any time
      void Print(const int width, const int height); //Print the current window to a file

      //Event cache status
      std::future<src::Source::metadata>& NextEventStatus(); //Get status of next event processing
      size_t EventCacheSize() const; //Get current number of events that are either in processing or ready
      size_t MaxEventCacheSize() const; //Get the maximum size of the event cache as configured by the user
      src::Source::metadata CurrentEvent() const; //Get the current event

      virtual void Render(const int width, const int height, const ImGuiIO& ioState); //Render this window
    
    private:
      //Encapsulate major setup steps
      void make_scenes();
      void reconfigure(std::unique_ptr<YAML::Node>&& config);

      //Configuration
      std::unique_ptr<YAML::Node> fConfig; //Configuration file for this job
      size_t fMaxEventCacheSize; //Don't let the event cache grow any bigger than this

      //Child Widgets
      mygl::Viewer fViewer;

      //Source of events
      std::unique_ptr<src::Source> fSource;

      //Delegate event processing to plugins below
      void ReadGeo();
      void ReadEvent();

      //Resources used by all plugins
      draw::Services fServices;

      //plugins
      std::vector<std::unique_ptr<draw::GeoControllerBase>> fGlobalDrawers; //Run for every file
      std::vector<std::unique_ptr<draw::EventControllerBase>> fEventDrawers; //Run for event event
      std::vector<std::unique_ptr<draw::CameraConfig>> fCameraConfigs; //Register camera(s) for every event
      //std::vector<std::unique_ptr<draw::ExternalDrawer>> fExtDrawers;

      //Event processing status
      std::queue<std::future<src::Source::metadata>> fEventCache; //Events in processing and that are ready to be loaded
      src::Source::metadata fCurrentEvent; //Source state when current event was first processed
  };
}

#endif //EVD_WINDOW  
