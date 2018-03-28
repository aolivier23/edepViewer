//File: MCHitDrawer.h
//Brief: MCHitDrawer is an ExternalDrawer to work with my edepsim viewer.  It draws MCHits as Cubes using PolyMesh.  
//Author: Andrew Olivier aolivier@ur.rochester.edu

//persistency includes
#include "persistency/MCHit.h"

//draw includes
#include "external/ExternalDrawer.cpp"

//external evd includes
#include "util/Palette.cpp"

//tinyxml2 include for configuration
#include <tinyxml2.h>

//ROOT includes
#include "TTreeReaderArray.h"

//c++ includes
#include <memory>

#ifndef DRAW_MCHITDRAWER_H
#define DRAW_MCHITDRAWER_H

namespace mygl
{
  class ColRecord;
  class TreeModel;
}

namespace mygl
{
  class MCHitDrawer: public draw::ExternalDrawer
  {
    public:
      MCHitDrawer(const tinyxml2::XMLElement* config);
      virtual ~MCHitDrawer() = default;

      virtual void ConnectTree(TTreeReader& reader) override;

      class MCHitRecord: public ColRecord
      {
        public:
          MCHitRecord(): ColRecord(), fEnergy("Energy [MeV]"), fTime("Time [ns]"), fParticle("Particle")
          {
            Add(fEnergy);
            Add(fTime);
            Add(fParticle);
          }
  
          mygl::TreeModel::Column<double> fEnergy; //The energy in this MCHit
          mygl::TreeModel::Column<double> fTime; //The time of this MCHit
          mygl::TreeModel::Column<std::string> fParticle; //The most direct cause of this MCHit.  Could be the names of multiple particles
      };
      std::shared_ptr<MCHitRecord> fHitRecord;

    protected:
      virtual void doRequestScenes(mygl::Viewer& viewer) override;
      virtual void doDrawEvent(const TG4Event& event, mygl::Viewer& viewer, mygl::VisID& nextID, draw::Services& services) override;

    private:
      std::unique_ptr<TTreeReaderArray<pers::MCHit>> fHits;

      //Drawing data
      mygl::Palette fPalette; //Mapping from MCHit::Energy to color

      //Configuration data
      std::string fHitName; //The name of the branch from which I will read MCHits.  
  };
}

#endif //DRAW_MCHITDRAWER_H
