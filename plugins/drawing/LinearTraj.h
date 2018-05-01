//File: LinearTraj.h
//Brief: A plugin that draws the true trajectories of particles by linearly extrapolating between trajectory points.  
//       Doesn't know anything about EM fields, but does know about fiducial volumes.  
//Author: Andrew Olivier aolivier@ur.rochester.edu

//draw includes
#include "plugins/drawing/EventDrawer.cpp"

//ROOT includes
#include "TDatabasePDG.h"

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
      LinearTraj(const YAML::Node& config);
      virtual ~LinearTraj() = default;

    protected:
      virtual void doRequestScenes(mygl::Viewer& viewer) override;
      virtual void doDrawEvent(const TG4Event& evt, mygl::Viewer& viewer, 
                               mygl::VisID& nextID, Services& services) override;

      //Drawing data
      float fLineWidth; //Width of lines to draw

      //Helper functions for drawing trajectories and trajectory points
      void AppendTrajectory(mygl::Viewer& scene, mygl::VisID& nextID, const mygl::TreeModel::iterator parent, 
                            const TG4Trajectory& traj, std::map<int, std::vector<TG4Trajectory>>& parentToTraj, 
                            Services& services);

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
  };
}

//TODO: Factory macro when I get around to writing the factory
#endif //DRAW_LINEARTRAJ_H
