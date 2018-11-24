//File: EventDrawer.cpp
//Brief: Interface between drawing code and event display window.  A EventDrawer is given a chance to request one or more 
//       Scene names from a Viewer.  Then, the main window tells the input provider to give the EventDrawer a source of data 
//       to draw, and the EventDrawer is given a chance to remove old objects and add new objects to its Scene(s).  
//       EventDrawer is the abstract base class for all plugins that can be used with the edepsim event display. 
//TODO: Make a EventDrawer responsible for exactly one Scene?  This would mean separate loops for trajectory point 
//      and trajectory drawing with my current design. 
//Author: Andrew Olivier aolivier@ur.rochester.edu

//gl includes
#include "gl/Viewer.h"

//yaml-cpp includes
#include "yaml-cpp/yaml.h"

//draw includes
#include "plugins/drawing/Services.cpp"

#ifndef DRAW_EVENTDRAWER_CPP
#define DRAW_EVENTDRAWER_CPP

class TG4Event;

namespace draw
{
  class EventDrawer
  {
    protected:
      using model_t = ctrl::SceneModel<std::unique_ptr<mygl::Drawable>>;

    public:
      EventDrawer(const YAML::Node& config): fDefaultDraw(true)
      {
        if(config["Draw"]) fDefaultDraw = config["Draw"].as<bool>();
      }
                          
      virtual ~EventDrawer() = default;

      virtual void RequestScenes(mygl::Viewer& viewer) //Request the needed Scene(s) from the Viewer
      {
        doRequestScenes(viewer);
      }

      //Produce a mapping from 3D objects to metadata for this event
      std::unique_ptr<model_t> DrawEvent(const TG4Event& evt, Services& services) 
      {
        return doDrawEvent(evt, services);
      }

    protected:
      //Provide a public interface, but call protected interface functions so that I can add 
      //behavior common to all EventDrawers here.
      virtual void doRequestScenes(mygl::Viewer& viewer) = 0;
      virtual std::unique_ptr<model_t> doDrawEvent(const TG4Event& evt, Services& services) = 0;

      //Disable drawing of a Scene by default
      bool fDefaultDraw;
  };
}

#endif //DRAW_EVENTDRAWER_CPP 
