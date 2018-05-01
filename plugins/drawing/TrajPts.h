//File: TrajPts.h
//Brief: A plugin that draws the true trajectories of particles by linearly extrapolating between trajectory points.  
//       Doesn't know anything about EM fields, but does know about fiducial volumes.  
//Author: Andrew Olivier aolivier@ur.rochester.edu

//draw includes
#include "plugins/drawing/EventDrawer.cpp"

//ROOT includes
#include "TDatabasePDG.h"

#ifndef DRAW_TRAJPTS_H
#define DRAW_TRAJPTS_H

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
  class TrajPts: public EventDrawer
  {
    public:
      //TODO: Configuration information when I implement a configuration system
      TrajPts(const YAML::Node& config);
      virtual ~TrajPts() = default;

    protected:
      virtual void doRequestScenes(mygl::Viewer& viewer) override;
      virtual void doDrawEvent(const TG4Event& evt, mygl::Viewer& viewer, 
                               mygl::VisID& nextID, Services& services) override;

      //Drawing data
      float fPointRad; //Radius of points to draw

      //Helper functions for drawing trajectories and trajectory points
      void AppendTrajPts(mygl::Viewer& scene, mygl::VisID& nextID,
                            const TG4Trajectory& traj, std::map<int, std::vector<TG4Trajectory>>& parentToTraj, 
                            const mygl::TreeModel::iterator ptRow, Services& services);

      mygl::TreeModel::iterator AddTrajPt(mygl::Viewer& scene, mygl::VisID& nextID, const std::string& particle, 
                                    const TG4TrajectoryPoint& pt, const mygl::TreeModel::iterator ptRow, const glm::vec4& color);

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
#endif //DRAW_TRAJPTS_H
