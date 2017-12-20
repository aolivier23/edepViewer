//File: EventDrawer.cpp
//Brief: Interface between drawing code and event display window.  A EventDrawer is given a chance to request one or more 
//       Scene names from a Viewer.  Then, the main window tells the input provider to give the EventDrawer a source of data 
//       to draw, and the EventDrawer is given a chance to remove old objects and add new objects to its Scene(s).  
//       EventDrawer is the abstract base class for all plugins that can be used with the edepsim event display. 
//TODO: Make a EventDrawer responsible for exactly one Scene?  This would mean separate loops for trajectory point 
//      and trajectory drawing with my current design. 
//Author: Andrew Olivier aolivier@ur.rochester.edu

//gl includes
#include "gl/VisID.h"
#include "gl/Viewer.h"

//draw includes
#include "plugins/drawing/Services.cpp"

#ifndef DRAW_EVENTDRAWER_CPP
#define DRAW_EVENTDRAWER_CPP

class TG4Event;

namespace draw
{
  class EventDrawer
  {
    public:
      EventDrawer() = default; //Configure a new EventDrawer
                          //TODO: Standardized configuration based on XML
      virtual ~EventDrawer() = default;

      virtual void RequestScenes(mygl::Viewer& viewer) //Request the needed Scene(s) from the Viewer
      {
        doRequestScenes(viewer);
      }

      void DrawEvent(const TG4Event& data, mygl::Viewer& viewer, mygl::VisID& nextID, Services& services) 
      //Remove old objects and draw new ones
      {
        doDrawEvent(data, viewer, nextID, services);
      }

    protected:
      //Provide a public interface, but call protected interface functions so that I can add 
      //behavior common to all EventDrawers here.
      virtual void doRequestScenes(mygl::Viewer& viewer) = 0;
      virtual void doDrawEvent(const TG4Event& data, mygl::Viewer& viewer, 
                               mygl::VisID& nextID, Services& services) = 0;
  };
}

#endif //DRAW_EVENTDRAWER_CPP 
