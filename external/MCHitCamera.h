//File: MCHitCamera.h
//Brief: MCHitCamera is an ExternalDrawer to work with my edepsim viewer.  It draws MCHits as Cubes using PolyMesh.  
//Author: Andrew Olivier aolivier@ur.rochester.edu

//persistency includes
#include "persistency/MCHit.h"

//draw includes
#include "external/ExternalDrawer.cpp"

//external evd includes
#include "util/Palette.cpp"

//ROOT includes
#include "TTreeReaderArray.h"

//c++ includes
#include <memory>

#ifndef DRAW_MCHITCAMERA_H
#define DRAW_MCHITCAMERA_H

namespace mygl
{
  class ColRecord;
  class TreeModel;
}

namespace mygl
{
  class MCHitCamera: public draw::ExternalDrawer
  {
    public:
      MCHitCamera(const YAML::Node& config);
      virtual ~MCHitCamera() = default;

      virtual void ConnectTree(TTreeReader& reader) override;

    protected:
      virtual void doRequestScenes(mygl::Viewer& viewer) override;
      virtual void doDrawEvent(const TG4Event& event, mygl::Viewer& viewer, mygl::VisID& nextID, draw::Services& services) override;

    private:
      std::unique_ptr<TTreeReaderArray<pers::MCHit>> fHits;

      //Configuration data
      std::string fHitName; //The name of the branch from which I will read MCHits.  
  };
}

#endif //DRAW_MCHITCAMERA_H
