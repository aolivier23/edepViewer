//File: Model.cpp
//Brief: Abstract base class for event display logic that maps data, MC, geometry, and other information to 
//       Drawables and Gtk::TreeStores.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//model includes
#include "model/Model.h"

namespace model
{
  //This file is mainly here to make the build system happy
  Model::Model(): fGeoTree(), fEvtTree()
  {
  }
}

