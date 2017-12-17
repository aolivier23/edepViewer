//File: LinearTraj.h
//Brief: A plugin that draws the ROOT geometry for the edepsim display.  Takes a TGeoManager as drawing data and 
//       draws 3D shapes using ROOT's tesselation facilities.  
//Author: Andrew Olivier aolivier@ur.rochester.edu

//draw includes
#include "plugins/drawing/EventDrawer.cpp"
#include "plugins/drawing/FiducialDrawer.cpp"

//ROOT includes
#include "TGeoManager.h" //For data
#include "TDatabasePDG.h"

//Gtkmm includes
#include <gtkmm.h>

//tinyxml2 include for configuration
#include <tinyxml2.h>

#ifndef DRAW_LINEARTRAJ_H
#define DRAW_LINEARTRAJ_H

namespace mygl
{
  class ColRecord;
  class VisID;
}

class TG4Event;
class TG4Trajectory;
class TG4TrajectoryPoint;

namespace draw
{
  class LinearTraj: public FiducialDrawer
  {
    public:
      //TODO: Configuration information when I implement a configuration system
      LinearTraj(const tinyxml2::XMLElement* config);
      virtual ~LinearTraj() = default;

    protected:
      virtual void doRequestScenes(mygl::Viewer& viewer) override;
      virtual void doDrawEvent(const TG4Event& evt, const TGeoManager& data, mygl::Viewer& viewer, 
                               mygl::VisID& nextID, Services& services) override;

      //Drawing data
      float fPointRad; //Radius of points to draw
      float fLineWidth; //Width of lines to draw

      //Helper functions for drawing trajectories and trajectory points
      void AppendTrajectory(mygl::Viewer& viewer, const TGeoManager& man, mygl::VisID& nextID, const Gtk::TreeModel::Row& parent, 
                            const TG4Trajectory& traj, std::map<int, std::vector<TG4Trajectory>>& parentToTraj, 
                            const Gtk::TreeModel::Row& ptRow, Services& services);

      Gtk::TreeModel::Row AddTrajPt(mygl::Viewer& viewer, const TGeoManager& man, mygl::VisID& nextID, const std::string& particle, 
                                    const TG4TrajectoryPoint& pt, const Gtk::TreeModel::Row& ptRow, const glm::vec4& color);

      //Description of the data saved for a trajectory
      class TrajRecord: public mygl::ColRecord
      {
        public:
          TrajRecord(): ColRecord()
          {
            add(fPartName);
            add(fEnergy);
          }

          Gtk::TreeModelColumn<std::string> fPartName;
          Gtk::TreeModelColumn<double> fEnergy;
      };

      TrajRecord fTrajRecord;

      //Description of the data saved for a TrajectoryPoint
      class TrajPtRecord: public mygl::ColRecord
      {
        public:
          TrajPtRecord(): ColRecord()
          {
            add(fMomMag);
            add(fTime);
            add(fProcess);
            add(fParticle);
          }
 
          Gtk::TreeModelColumn<double> fMomMag;
          Gtk::TreeModelColumn<double> fTime;
          Gtk::TreeModelColumn<std::string> fProcess;
          Gtk::TreeModelColumn<std::string> fParticle;
      };
      TrajPtRecord fTrajPtRecord;
  };
}

//TODO: Factory macro when I get around to writing the factory
#endif //DRAW_LINEARTRAJ_H
