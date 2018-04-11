//File: MCHitCamera.cpp
//Brief: An MCHitCamera is an ExternalDrawer for my edepsim display that creates a PlaneCamera zoomed in to just see all of the MCHits in the event.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//Core IMGUI functions
#include "imgui.h"

//gl includes
#include "gl/camera/PlaneCam.h"

//plugin includes
#include "plugins/Factory.cpp"

//edepsim includes
#include "TG4Event.h"

//local includes
#include "external/MCHitCamera.h"

namespace mygl
{
  MCHitCamera::MCHitCamera(const tinyxml2::XMLElement* config): ExternalDrawer(), fHits(nullptr), fHitName("NeutronHits")
  {
    const auto hitName = config->Attribute("HitName");
    if(hitName != nullptr) fHitName = hitName;
  }

  void MCHitCamera::ConnectTree(TTreeReader& reader)
  {
    std::cout << "Connecting MCHitsDrawer to TTreeReader.\n";
    fHits.reset(new TTreeReaderArray<pers::MCHit>(reader, fHitName.c_str())); //TODO: Get name of hits to process from XML file in constructor
  }

  void MCHitCamera::doRequestScenes(mygl::Viewer& viewer)
  {
    //TODO: Define pos and dir here.  Really, this is just a dummy camera that will be replaced when the first event is drawn.
    viewer.AddCamera("MCHits", std::unique_ptr<mygl::PlaneCam>(new mygl::PlaneCam(glm::vec3(), glm::vec3(), glm::vec3(0.0, 1.0, 0.0), 10000., 100.)));
    viewer.MakeCameraCurrent("MCHits");
  }

  void MCHitCamera::doDrawEvent(const TG4Event& event, mygl::Viewer& viewer, mygl::VisID& nextID, draw::Services& services)
  {
    viewer.RemoveCamera("MCHits");

    //Find bounding box for all MCHits in this event
    const auto large = std::numeric_limits<float>::max();
    float minX = large, maxX = -large, minY = large, maxY = -large, minZ = large, maxZ = -large;
    glm::vec3 center;
    for(const auto& hit: *fHits)
    {
      center.x += hit.Position.X();
      center.y += hit.Position.Y();
      center.z += hit.Position.Z();

      if(hit.Position.X() < minX) minX = hit.Position.X();
      if(hit.Position.X() > maxX) maxX = hit.Position.X();
 
      if(hit.Position.Y() < minY) minY = hit.Position.Y();
      if(hit.Position.Y() > maxY) maxY = hit.Position.Y();

      if(hit.Position.Z() < minZ) minZ = hit.Position.Z();
      if(hit.Position.Z() > maxZ) maxZ = hit.Position.Z();
    }
    center *= 1./fHits->GetSize();

    float zoom = 1.0; //Reasonable default in case there are no MCHits

    if(fHits->GetSize() > 0)
    {
      //Get window width and height to calculate zoom I want to use
      const float width = ImGui::GetIO().DisplaySize.x;
      const float height = ImGui::GetIO().DisplaySize.y;
      std::cout << "width is " << width << ".  Height is " << height << "\n";

      float xDist = maxX-minX;
      if(xDist == 0) xDist = fHits->At(0).Width*10.; //Multiply by 10 so I can still see other things
      std::cout << "xDist is " << xDist << "\n";
      float yDist = maxY-minY;    
      if(yDist == 0) yDist = fHits->At(0).Width*10.; //Multiply by 10 so I can still see other things
      std::cout << "yDist is " << yDist << "\n";
      zoom = std::min(width/2./xDist, height/2./yDist);
    }

    glm::vec3 dir(0., 0., 0.);

    viewer.AddCamera("MCHits", std::unique_ptr<mygl::PlaneCam>(new mygl::PlaneCam(center, glm::normalize(dir), glm::vec3(0.0, 1.0, 0.0), maxZ, 100., zoom))); 
    viewer.MakeCameraCurrent("MCHits");
  }

  REGISTER_PLUGIN(MCHitCamera, draw::ExternalDrawer); 
}
