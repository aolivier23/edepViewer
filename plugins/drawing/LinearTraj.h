//File: LinearTraj.h
//Brief: A plugin that draws the true trajectories of particles by linearly extrapolating between trajectory points.  
//       Doesn't know anything about EM fields, but does know about fiducial volumes.  
//Author: Andrew Olivier aolivier@ur.rochester.edu

//draw includes
#include "plugins/drawing/EventDrawer.cpp"

//ROOT includes
#include "TDatabasePDG.h"

//tinyxml2 include for configuration
#include <tinyxml2.h>

#ifndef DRAW_LINEARTRAJ_H
#define DRAW_LINEARTRAJ_H

namespace mygl
{
  class ColRecord;
  class VisID;
  class TreeModel;
}

class TG4Event;
class TG4Trajectory;
class TG4TrajectoryPoint;

namespace draw
{
  class LinearTraj: public EventDrawer
  {
    public:
      //TODO: Configuration information when I implement a configuration system
      LinearTraj(const tinyxml2::XMLElement* config);
      virtual ~LinearTraj() = default;

    protected:
      virtual void doRequestScenes(mygl::Viewer& viewer) override;
      virtual void doDrawEvent(const TG4Event& evt, mygl::Viewer& viewer, 
                               mygl::VisID& nextID, Services& services) override;

      //Drawing data
      float fPointRad; //Radius of points to draw
      float fLineWidth; //Width of lines to draw

      //Helper functions for drawing trajectories and trajectory points
      void AppendTrajectory(mygl::Viewer& viewer, mygl::VisID& nextID, const mygl::TreeModel::iterator parent, 
                            const TG4Trajectory& traj, std::map<int, std::vector<TG4Trajectory>>& parentToTraj, 
                            const mygl::TreeModel::iterator ptRow, Services& services);

      mygl::TreeModel::iterator AddTrajPt(mygl::Viewer& viewer, mygl::VisID& nextID, const std::string& particle, 
                                    const TG4TrajectoryPoint& pt, const mygl::TreeModel::iterator ptRow, const glm::vec4& color);

      //Description of the data saved for a trajectory
      class TrajRecord: public mygl::ColRecord
      {
        public:
          TrajRecord(): ColRecord(), fPartName("Name"), fEnergy("Energy [MeV]")
          {
            Add(fPartName);
            Add(fEnergy);
          }

          mygl::TreeModel::Column<std::string> fPartName;
          mygl::TreeModel::Column<double> fEnergy;
      };

      std::shared_ptr<TrajRecord> fTrajRecord;

      //Description of the data saved for a TrajectoryPoint
      class TrajPtRecord: public mygl::ColRecord
      {
        public:
          TrajPtRecord(): ColRecord(), fMomMag("Momentum [MeV/c]"), fTime("Time [ns]"), fProcess("Process"), fParticle("Particle")
          {
            Add(fMomMag);
            Add(fTime);
            Add(fProcess);
            Add(fParticle);
          }
 
          mygl::TreeModel::Column<double> fMomMag;
          mygl::TreeModel::Column<double> fTime;
          mygl::TreeModel::Column<std::string> fProcess;
          mygl::TreeModel::Column<std::string> fParticle;
      };
      std::shared_ptr<TrajPtRecord> fTrajPtRecord;
  };
}

//TODO: Factory macro when I get around to writing the factory
#endif //DRAW_LINEARTRAJ_H
