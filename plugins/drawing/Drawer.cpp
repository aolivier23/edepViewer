//File: Drawer.cpp
//Brief: Interface between drawing code and event display window.  A Drawer is given a chance to request one or more 
//       Scene names from a Viewer.  Then, the main window tells the input provider to give the Drawer a source of data 
//       to draw, and the Drawer is given a chance to remove old objects and add new objects to its Scene(s).  
//       Drawer is the abstract base class for all plugins that can be used with the edepsim event display. 
//TODO: Make a Drawer responsible for exactly one Scene?  This would mean separate loops for trajectory point 
//      and trajectory drawing with my current design. 
//Author: Andrew Olivier aolivier@ur.rochester.edu

//gl includes
#include "gl/VisID.h"
#include "gl/Viewer.h"

#ifndef DRAW_DRAWER_CPP
#define DRAW_DRAWER_CPP

namespace draw
{
  template <class DATA>
  class Drawer
  {
    public:
      Drawer() = default; //Configure a new Drawer
                          //TODO: Standardized configuration based on XML
      virtual ~Drawer() = default;

      virtual void RequestScenes(mygl::Viewer& viewer) //Request the needed Scene(s) from the Viewer
      {
        doRequestScenes(viewer);
      }

      void DrawEvent(const DATA& data, mygl::Viewer& viewer, mygl::VisID& nextID) //Remove old objects and draw new ones
      {
        doDrawEvent(data, viewer, nextID);
      }

    protected:
      //Provide a public interface, but call protected interface functions so that I can add 
      //behavior common to all Drawers here.
      virtual void doRequestScenes(mygl::Viewer& viewer) = 0;
      virtual void doDrawEvent(const DATA& data, mygl::Viewer& viewer, mygl::VisID& nextID) = 0;
  };
}

#endif //DRAW_DRAWER_CPP 
