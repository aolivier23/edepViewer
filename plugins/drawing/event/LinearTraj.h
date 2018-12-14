//File: LinearTraj.h
//Brief: A plugin that draws the true trajectories of particles by linearly extrapolating between trajectory points.  
//       Doesn't know anything about EM fields, but does know about fiducial volumes.  
//Author: Andrew Olivier aolivier@ur.rochester.edu

//draw includes
#include "EventController.cpp"
#include "gl/metadata/Column.cpp"

#ifndef DRAW_LINEARTRAJ_H
#define DRAW_LINEARTRAJ_H

class TG4Event;
class TG4Trajectory;
class TG4TrajectoryPoint;

namespace draw
{
  class LinearTraj
  {
    public:
      //TODO: Configuration information when I implement a configuration system
      LinearTraj(const YAML::Node& config);
      virtual ~LinearTraj() = default;

      legacy::scene_t& doRequestScene(mygl::Viewer& viewer);
      std::unique_ptr<legacy::model_t> doDraw(const TG4Event& evt, Services& services);

    private:
      //Drawing data
      float fLineWidth; //Width of lines to draw
      bool fDefaultDraw; //Draw this scene by default?

      //Helper functions for drawing trajectories and trajectory points
      void AppendTrajectory(legacy::model_t::view& parent, const TG4Trajectory& traj, 
                            std::map<int, std::vector<TG4Trajectory>>& parentToTraj, Services& services);

      //Description of the data saved for a trajectory
      class TrajRecord: public ctrl::ColumnModel
      {
        public:
          TrajRecord(): ColumnModel(), fPartName("Name"), fEnergy("Energy [MeV]")
          {
            Add(fPartName);
            Add(fEnergy);
          }

          ctrl::Column<std::string> fPartName;
          ctrl::Column<double> fEnergy;
      };

      std::shared_ptr<TrajRecord> fTrajRecord;
  };
}
#endif //DRAW_LINEARTRAJ_H
