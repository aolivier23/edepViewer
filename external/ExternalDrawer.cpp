//File: ExternalDrawer.cpp
//Brief: Interface between drawing code and event display window.  Like EventDrawer ExternalDrawer is given a chance to request one or more 
//       Scene names from a Viewer. ExternalDrawer will not be given a TG4Event.  Instead, it gets a chance to connect to new TTreeReaders from 
//       which it must know how to extract the objects it needs.   
//       ExternalDrawer is the abstract base class for all plugins that can be used with the edepsim event display to process objects that are 
//       not from edepsim.  If you wrote your own metadata class that you want to stick into edepsim files, you are in the right place.  
//Author: Andrew Olivier aolivier@ur.rochester.edu

//gl includes
#include "gl/VisID.h"
#include "gl/Viewer.h"

//draw includes
#include "plugins/drawing/Services.cpp"

//ROOT includes
#include "TTreeReader.h"

#ifndef DRAW_EXTERNALDRAWER_CPP
#define DRAW_EXTERNALDRAWER_CPP

class TG4Event;

namespace draw
{
  class ExternalDrawer
  {
    public:
      ExternalDrawer() = default; //Configure a new ExternalDrawer
      virtual ~ExternalDrawer() = default;

      virtual void RequestScenes(mygl::Viewer& viewer) //Request the needed Scene(s) from the Viewer
      {
        doRequestScenes(viewer);
      }

      virtual void ConnectTree(TTreeReader& reader) = 0; //Connect to a TTreeReader.  Curently, I plan to just call this once in a Source's lifetime.

      void DrawEvent(const TG4Event& event, mygl::Viewer& viewer, mygl::VisID& nextID, Services& services) 
      //Remove old objects and draw new ones
      {
        doDrawEvent(event, viewer, nextID, services);
      }

    protected:
      //Provide a public interface, but call protected interface functions so that I can add 
      //behavior common to all ExternalDrawers here.
      virtual void doRequestScenes(mygl::Viewer& viewer) = 0;
      virtual void doDrawEvent(const TG4Event& event, mygl::Viewer& viewer, 
                               mygl::VisID& nextID, Services& services) = 0;
  };
}

#endif //DRAW_EXTERNALDRAWER_CPP 
