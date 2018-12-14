//File: TrajPts.h
//Brief: A plugin that draws the true trajectories of particles by linearly extrapolating between trajectory points.  
//       Doesn't know anything about EM fields, but does know about fiducial volumes.  
//Author: Andrew Olivier aolivier@ur.rochester.edu

//draw includes
#include "EventController.cpp"

//gl includes
#include "gl/metadata/Column.cpp"

#ifndef DRAW_TRAJPTS_H
#define DRAW_TRAJPTS_H

class TG4Event;
class TG4Trajectory;
class TG4TrajectoryPoint;

namespace draw
{
  class TrajPts
  {
    public:
      //TODO: Configuration information when I implement a configuration system
      TrajPts(const YAML::Node& config);
      virtual ~TrajPts() = default;

      legacy::scene_t& doRequestScene(mygl::Viewer& viewer);
      std::unique_ptr<legacy::model_t> doDraw(const TG4Event& evt, Services& services);

    private:
      //Drawing data
      float fPointRad; //Radius of points to draw
      bool fDefaultDraw; //Draw this scene by default?

      //Helper functions for drawing trajectories and trajectory points
      void AppendTrajPts(legacy::model_t::view& parent, const TG4Trajectory& traj,
                         std::map<int, std::vector<TG4Trajectory>>& parentToTraj, Services& services);

      legacy::model_t::view AddTrajPt(legacy::model_t::view& parent, const std::string& particle, 
                                      const TG4TrajectoryPoint& pt, const glm::vec4& color);

      //Description of the data saved for a TrajectoryPoint
      class TrajPtRecord: public ctrl::ColumnModel
      {
        public:
          TrajPtRecord(): ColumnModel(), fMomMag("Momentum [MeV/c]"), fTime("Time [ns]"), fProcess("Process"), fParticle("Particle")
          {
            Add(fMomMag);
            Add(fTime);
            Add(fProcess);
            Add(fParticle);
          }
 
          ctrl::Column<double> fMomMag;
          ctrl::Column<double> fTime;
          ctrl::Column<std::string> fProcess;
          ctrl::Column<std::string> fParticle;
      };
      std::shared_ptr<TrajPtRecord> fTrajPtRecord;
  };
}
#endif //DRAW_TRAJPTS_H
