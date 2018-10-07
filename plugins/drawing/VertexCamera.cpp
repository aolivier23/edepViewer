//File: VertexCamera.cpp
//Brief: Interface between drawing code and event display window.  A VertexCamera creates a PlaneCam zoomed in on each primary interaction vertex.  
//Author: Andrew Olivier aolivier@ur.rochester.edu

//plugin includes
#include "plugins/drawing/VertexCamera.h"
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
  VertexCamera::VertexCamera(const YAML::Node& config): EventDrawer(config), fFirstTime(true)
  {
    //TODO: Configuration?    
  }

  void VertexCamera::RemoveAll(mygl::Viewer& /*viewer*/)
  {
    //Do nothing
  }

  void VertexCamera::doRequestScenes(mygl::Viewer& viewer)
  {
    //Nothing to do here?
  }

  void VertexCamera::doDrawEvent(const TG4Event& data, mygl::Viewer& viewer, mygl::VisID& nextID, Services& services)
  {
    //Remove old drawing elements
    if(fFirstTime)
    {
      fFirstTime = false;
    }
    else viewer.RemoveCamera("Interaction");

    TLorentzVector avgPos;
    for(const auto& vert: data.Primaries)
    { 
      #ifdef EDEPSIM_FORCE_PRIVATE_FIELDS
      avgPos += vert.GetPosition();
      #else
      avgPos += vert.Position;
      #endif
      //viewer.AddCamera(vert.Reaction, new PlaneCam()); //TODO: Configure new camera based on vert's position and maybe zoom based on some measure of activity?
    }
  
    const glm::vec3 pos(avgPos.X(), avgPos.Y(), avgPos.Z()); //0., 0., avgPos.Z());
    const glm::vec3 dir(avgPos.X() - pos.x, avgPos.Y() - pos.y, avgPos.Z() - pos.z);

    //Center the default camera on average position of interaction vertices in each event
    viewer.AddCamera("Interaction", std::unique_ptr<mygl::PlaneCam>(new mygl::PlaneCam(pos, glm::normalize(dir), glm::vec3(0.0, 1.0, 0.0), 10000., 100.)));
    if(fDefaultDraw) viewer.MakeCameraCurrent("Interaction");
  }

  REGISTER_PLUGIN(VertexCamera, EventDrawer);
}
