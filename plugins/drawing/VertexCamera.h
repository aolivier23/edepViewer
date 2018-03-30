//File: VertexCamera.cpp
//Brief: Interface between drawing code and event display window.  A VertexCamera creates a new Camera centered on each primary vertex in the event.  
//Author: Andrew Olivier aolivier@ur.rochester.edu

//plugin includes
#include "EventDrawer.cpp"

//util includes
#include "util/Palette.cpp"

//gl includes
#include "gl/VisID.h"
#include "gl/Viewer.h"

#ifndef DRAW_VERTEXCAMERA_CPP
#define DRAW_VERTEXCAMERA_CPP

class TG4Event;

namespace tinyxml2
{
  class XMLElement;
}

namespace mygl
{
  class TreeModel;
}

namespace draw
{
  class VertexCamera: public EventDrawer
  {
    public:
      VertexCamera(const tinyxml2::XMLElement* config);
      virtual ~VertexCamera() = default;

    protected:
      virtual void doRequestScenes(mygl::Viewer& viewer);
      virtual void doDrawEvent(const TG4Event& data, mygl::Viewer& viewer, 
                               mygl::VisID& nextID, Services& services);

      //TODO: This is an inelegant solution.  Probably indicates I should rewrite the Camera interface.
      bool fFirstTime;
  };
}

#endif //DRAW_EDEPDEDX_CPP 
