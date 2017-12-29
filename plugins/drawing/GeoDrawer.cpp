//File: GeoDrawer.cpp
//Brief: Interface between drawing code and event display window.  A GeoDrawer is given a chance to request one or more 
//       Scene names from a Viewer.  Then, the main window tells the input provider to give the GeoDrawer a source of data 
//       to draw, and the GeoDrawer is given a chance to remove old objects and add new objects to its Scene(s).  
//       GeoDrawer is the abstract base class for all plugins that can be used with the edepsim event display. 
//TODO: Make a GeoDrawer responsible for exactly one Scene?  This would mean separate loops for trajectory point 
//      and trajectory drawing with my current design. 
//Author: Andrew Olivier aolivier@ur.rochester.edu

//gl includes
#include "gl/VisID.h"
#include "gl/Viewer.h"

#ifndef DRAW_GEODRAWER_CPP
#define DRAW_GEODRAWER_CPP

class TGeoManager;
class TG4Event;

namespace draw
{
  class GeoDrawer
  {
    public:
      GeoDrawer() = default; //Configure a new GeoDrawer
      virtual ~GeoDrawer() = default;

      virtual void RequestScenes(mygl::Viewer& viewer) //Request the needed Scene(s) from the Viewer
      {
        doRequestScenes(viewer);
      }

      void DrawEvent(const TGeoManager& man, mygl::Viewer& viewer, mygl::VisID& nextID) //Remove old objects and draw new ones
      {
        doDrawEvent(man, viewer, nextID);
      }

    protected:
      //Provide a public interface, but call protected interface functions so that I can add 
      //behavior common to all GeoDrawers here.
      virtual void doRequestScenes(mygl::Viewer& viewer) = 0;
      virtual void doDrawEvent(const TGeoManager& man, mygl::Viewer& viewer, mygl::VisID& nextID) = 0;
  };
}

#endif //DRAW_GEODRAWER_CPP 
