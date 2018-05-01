//File: Grids.cpp
//Brief: Interface between drawing code and event display window.  A Grids is given a chance to request one or more 
//       Scene names from a Viewer.  Then, the main window tells the input provider to give the Grids a source of data 
//       to draw, and the Grids is given a chance to remove old objects and add new objects to its Scene(s).  
//       Grids is the abstract base class for all plugins that can be used with the edepsim event display. 
//TODO: Make a Grids responsible for exactly one Scene?  This would mean separate loops for trajectory point 
//      and trajectory drawing with my current design. 
//Author: Andrew Olivier aolivier@ur.rochester.edu

//Base class
#include "plugins/drawing/GeoDrawer.cpp"

//gl includes
#include "gl/VisID.h"
#include "gl/Viewer.h"

#ifndef DRAW_GRIDS_H
#define DRAW_GRIDS_H

class TGeoManager;

namespace mygl
{
  class TreeModel;
}

namespace draw
{
  class Grids: public GeoDrawer
  {
    public:
      Grids(const YAML::Node& config); //Configure a new Grids
      virtual ~Grids() = default;

    protected:
      //Provide a public interface, but call protected interface functions so that I can add 
      //behavior common to all Grids here.
      virtual void doRequestScenes(mygl::Viewer& viewer);
      virtual void doDrawEvent(const TGeoManager& man, mygl::Viewer& viewer, mygl::VisID& nextID);

      float fLineWidth;

      class GuideRecord: public mygl::ColRecord
      {
        public:
          GuideRecord(): ColRecord(), fName("Name")
          {
            Add(fName);
          }

          mygl::TreeModel::Column<std::string> fName;
      };

      std::shared_ptr<GuideRecord> fGuideRecord;
  };
}

#endif //DRAW_GRIDS_H
