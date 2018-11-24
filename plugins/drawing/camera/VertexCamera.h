//File: VertexCamera.cpp
//Brief: Interface between drawing code and event display window.  A VertexCamera creates a new Camera centered on each primary vertex in the event.  
//Author: Andrew Olivier aolivier@ur.rochester.edu

//plugin includes
#include "CameraConfig.cpp"

//gl includes
#include "gl/Viewer.h"

#ifndef DRAW_VERTEXCAMERA_H
#define DRAW_VERTEXCAMERA_H

class TG4Event;

namespace mygl
{
  class TreeModel;
}

namespace draw
{
  class VertexCamera: public CameraConfig
  {
    public:
      VertexCamera(const YAML::Node& config);
      virtual ~VertexCamera() = default;

    protected:
      using CameraConfig::map_t;
      virtual map_t doMakeCameras(const TG4Event& evt, Services& services) override;
  };
}

#endif //DRAW_VERTEXCAMERA_H
