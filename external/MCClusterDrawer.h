//File: MCClusterDrawer.h
//Brief: MCClusterDrawer is an ExternalDrawer to work with my edepsim viewer.  It draws MCClusters as Cubes using PolyMesh.  
//Author: Andrew Olivier aolivier@ur.rochester.edu

//persistency includes
#include "persistency/MCCluster.h"
#include "persistency/MCHit.h"

//draw includes
#include "external/ExternalDrawer.cpp"

//external evd includes
#include "util/Palette.cpp"

//ROOT includes
#include "TTreeReaderArray.h"

//c++ includes
#include <memory>

#ifndef DRAW_MCCLUSTERDRAWER_H
#define DRAW_MCCLUSTERDRAWER_H

namespace mygl
{
  class ColRecord;
}

namespace mygl
{
  class MCClusterDrawer: public draw::ExternalDrawer
  {
    public:
      MCClusterDrawer(const YAML::Node& config);
      virtual ~MCClusterDrawer() = default;

      virtual void ConnectTree(TTreeReader& reader) override;

      virtual void RemoveAll(mygl::Viewer& viewer) override;

      //TODO: Why is this public?
      class MCClusterRecord: public ColRecord
      {
        public:
          MCClusterRecord(): ColRecord(), fEnergy("Energy [MeV]"), fTime("Time [ns]"), fParticle("Cause")
          {
            Add(fEnergy);
            Add(fTime);
            Add(fParticle);
          }
  
          TreeModel::Column<double> fEnergy; //The energy in this MCCluster
          TreeModel::Column<double> fTime; //The energy-weighted average time of this MCCluster
          TreeModel::Column<std::string> fParticle; //The most direct cause of this MCCluster.  Could be the names of multiple particles
      };
      std::shared_ptr<MCClusterRecord> fClusterRecord;

    protected:
      virtual void doRequestScenes(mygl::Viewer& viewer) override;
      virtual void doDrawEvent(const TG4Event& event, mygl::Viewer& viewer, mygl::VisID& nextID, draw::Services& services) override;

    private:
      std::unique_ptr<TTreeReaderArray<pers::MCCluster>> fClusters; //The MCClusters that will be drawn

      //Drawing data

      //Configuration data
      std::string fClusterName; //The name of the branch from which I will read MCClusters.  
  };
}

#endif //DRAW_MCCLUSTERDRAWER_H
