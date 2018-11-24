//File: ForceDependencyOnLibraries.h
//Brief: Include symbols from each plugin library to make sure they can't be optimized out.  
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef FORCEDEPENDENCYONLIBRARIES_H
#define FORCEDEPENDENCYONLIBRARIES_H

#include "plugins/drawing/geometry/GeoController.cpp"
#include "plugins/drawing/geometry/DefaultGeo.h"

#include "plugins/drawing/event/EventController.cpp"
#include "plugins/drawing/event/LinearTraj.h"

namespace
{
  YAML::Node node;
  static draw::DefaultGeo dummyGeo(node);
  static draw::LinearTraj traj(node);
}

#endif //FORCEDEPENDENCYONLIBRARIES_H
