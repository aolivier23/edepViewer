//File: ForceDependencyOnLibraries.h
//Brief: Include symbols from each plugin library to make sure they can't be optimized out.  
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef FORCEDEPENDENCYONLIBRARIES_H
#define FORCEDEPENDENCYONLIBRARIES_H

#include "plugins/drawing/geometry/DefaultGeo.h"

#include "plugins/drawing/event/LinearTraj.h"

#include "plugins/drawing/camera/VertexCamera.h"

namespace
{
  YAML::Node node;
  static draw::DefaultGeo dummyGeo(node);
  static draw::LinearTraj traj(node);
  static draw::VertexCamera config(node);
}

#endif //FORCEDEPENDENCYONLIBRARIES_H
