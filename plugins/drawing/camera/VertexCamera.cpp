//File: VertexCamera.cpp
//Brief: Interface between drawing code and event display window.  A VertexCamera creates a PlaneCam zoomed in on each primary interaction vertex.  
//Author: Andrew Olivier aolivier@ur.rochester.edu

//plugin includes
#include "VertexCamera.h"
#include "plugins/Factory.cpp"

//gl includes
#include "gl/Viewer.h"
#include "gl/camera/PlaneCam.h"

//ROOT includes
#include "TGeoManager.h"
#include "TLorentzVector.h"

//edepsim includes
#include "TG4Event.h"

//tinyxml2 include for configuration
#include <tinyxml2.h>

namespace draw
{
  VertexCamera::VertexCamera(const YAML::Node& config): CameraConfig(config)
  {
    //TODO: Configuration?    
  }

  VertexCamera::map_t VertexCamera::doMakeCameras(const TG4Event& data, Services& services)
  {
    TLorentzVector avgPos;
    for(const auto& vert: data.Primaries)
    { 
      #ifdef EDEPSIM_FORCE_PRIVATE_FIELDS
      avgPos += vert.GetPosition();
      #else
      avgPos += vert.Position;
      #endif
    }
  
    const glm::vec3 pos(avgPos.X(), avgPos.Y(), avgPos.Z()); 
    const glm::vec3 dir(avgPos.X() - pos.x, avgPos.Y() - pos.y, avgPos.Z() - pos.z);

    //Center the default camera on average position of interaction vertices in each event
    map_t retVal;
    retVal.emplace("Interaction", 
                   std::unique_ptr<mygl::PlaneCam>(new mygl::PlaneCam(pos, glm::normalize(dir), glm::vec3(0.0, 1.0, 0.0), 10000., 100.)));
    return retVal;
  }

  REGISTER_PLUGIN(VertexCamera, CameraConfig);
}
