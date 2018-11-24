//File: CameraConfig.cpp
//Brief: Plugin to create new views of an event's 3D objects based on the event itself.  
//       A CameraConfig doesn't work with SceneController at all.  Instead, it interacts 
//       directly with a Viewer (for now) to register Cameras to be drawn.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//gl includes
#include "gl/camera/Camera.h"

//yaml-cpp includes
#include "yaml-cpp/yaml.h"

//draw includes
#include "plugins/drawing/Services.cpp"

//c++ includes
#include <queue>

#ifndef DRAW_EVENTDRAWER_CPP
#define DRAW_EVENTDRAWER_CPP

class TG4Event;

namespace draw
{
  class CameraConfig
  {
    protected:
      using map_t = std::map<std::string, std::unique_ptr<mygl::Camera>>;

    public:
      CameraConfig(const YAML::Node& config) {}
      virtual ~CameraConfig() = default;

      //Produce a mapping from 3D objects to metadata for this event
      void MakeCameras(const TG4Event& evt, Services& services) 
      {
        fConfigCache.push(doMakeCameras(evt, services));
      }

      //Add the cached cameras to another map
      void AppendCameras(map_t& otherCameras)
      {
        auto& nextConfig = fConfigCache.front();
        for(auto& camera: nextConfig) otherCameras.emplace(camera.first, std::unique_ptr<mygl::Camera>(camera.second.release())); 
        //TODO: splice-like functionality in c++17?
        fConfigCache.pop();
      }

      //Clear cache of CameraConfigs
      void Clear()
      {
        fConfigCache = std::queue<map_t>();
      }

    protected:
      //Provide a public interface, but call protected interface functions so that I can add 
      //behavior common to all CameraConfigs here.
      virtual map_t doMakeCameras(const TG4Event& evt, Services& services) = 0;

    private:
      std::queue<map_t> fConfigCache; //Cache the most recent set of cameras for updating 
                                      //the Viewer once all plugins are ready.
  }; 
}

#endif //DRAW_EVENTDRAWER_CPP 
