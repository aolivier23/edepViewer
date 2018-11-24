//File: CameraConfig.cpp
//Brief: Plugin to create new views of an event's 3D objects based on the event itself.  
//       A CameraConfig doesn't work with SceneController at all.  Instead, it interacts 
//       directly with a Viewer (for now) to register Cameras to be drawn.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//gl includes
#include "gl/Viewer.h"

//yaml-cpp includes
#include "yaml-cpp/yaml.h"

//draw includes
#include "plugins/drawing/Services.cpp"

#ifndef DRAW_EVENTDRAWER_CPP
#define DRAW_EVENTDRAWER_CPP

class TG4Event;

namespace draw
{
  class CameraConfig
  {
    public:
      CameraConfig(const YAML::Node& config)
      {
      }
                          
      virtual ~CameraConfig() = default;

      //Produce a mapping from 3D objects to metadata for this event
      std::map<std::string, std::unique_ptr<Camera>> MakeCameras(const TG4Event& evt, Services& services) 
      {
        return doMakeCameras(evt, services);
      }

    protected:
      //Provide a public interface, but call protected interface functions so that I can add 
      //behavior common to all CameraConfigs here.
      virtual std::map<std::string, std::unique_ptr<Camera>> doMakeCameras(const TG4Event& evt, Services& services) = 0;
  };
}

#endif //DRAW_EVENTDRAWER_CPP 
