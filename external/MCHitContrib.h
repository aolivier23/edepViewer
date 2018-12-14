//File: MCHitContrib.h
//Brief: MCHitContrib is an ExternalDrawer to work with my edepsim viewer.  It draws MCHits as Cubes using PolyMesh.  
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

#ifndef DRAW_MCHITCONTRIB_H
#define DRAW_MCHITCONTRIB_H

namespace mygl
{
  class ColRecord;
  class TreeModel;
}

namespace mygl
{
  class MCHitContrib: public draw::ExternalDrawer
  {
    public:
      MCHitContrib(const YAML::Node& config);
      virtual ~MCHitContrib() = default;

      virtual void RemoveAll(mygl::Viewer& viewer) override;

      virtual void ConnectTree(TTreeReader& reader) override;

      virtual void Render() override;

      class MCHitRecord: public ColRecord
      {
        public:
          MCHitRecord(): ColRecord(), fEnergy("Energy [MeV]"), fTime("Time [ns]"), fDist("Vtx. Dist. [mm]"), fParticle("Particle")
          {
            Add(fEnergy);
            Add(fTime);
            Add(fDist);
            Add(fParticle);
          }
  
          mygl::TreeModel::Column<double> fEnergy; //The energy in this MCHit
          mygl::TreeModel::Column<double> fTime; //The time of this MCHit
          mygl::TreeModel::Column<double> fDist; //The distance of this MCHit from the vertex
          mygl::TreeModel::Column<std::string> fParticle; //The most direct cause of this MCHit.  Could be the names of multiple particles
      };
      std::shared_ptr<MCHitRecord> fHitRecord;

    protected:
      virtual void doRequestScenes(mygl::Viewer& viewer) override;
      virtual void doDrawEvent(const TG4Event& event, mygl::Viewer& viewer, mygl::VisID& nextID, draw::Services& services) override;

    private:
      std::unique_ptr<TTreeReaderArray<pers::MCHit>> fHits;

      //Drawing data
      struct FSInfo //Data I will need to create a nice legend of the FS particles I am drawing
      {
        int TrackID; //The Geant TrackID of this particle
        std::string Name; //Particle name from edep-sim
        double Energy; //Initial energy of this particle

        bool operator <(const FSInfo& other) const { return TrackID < other.TrackID; } //Necessary so that std::map has some way to sort these objects
      };

      std::map<FSInfo, glm::vec3> fContribToColor; //Mapping from FS particle to color.  I will use it to create a legend for what I'm drawing. 

      //Configuration data
      std::string fHitName; //The name of the branch from which I will read MCHits.  
  };
}

#endif //DRAW_MCHITCONTRIB_H
