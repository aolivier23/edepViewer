//File: Services.cpp
//Brief: A Services struct contains resources that are shared by all plugins while they are drawing. 
//       The application shall own a Services struct that it will pass to each plugin that needs it.
//       I have not gone out of my way to protect the user in multi-threaded applications in the 
//       Services struct.  Services are not copyable, and their members also cannot be copied by virtue of 
//       being unique_ptrs.
//Author: Andrew Olivier aolivier@ur.rochester.edu 

//util includes
#include "util/PDGToColor.h"
#include "util/Geometry.cpp"

//c++ includes
#include <memory>

#ifndef DRAW_SERVICES_CPP
#define DRAW_SERVICES_CPP

namespace draw
{
  struct Services
  {
    Services(): fPDGToColor(new mygl::PDGToColor()) {}

    std::unique_ptr<mygl::PDGToColor> fPDGToColor;
    std::unique_ptr<util::Geometry> fGeometry;
  };
}

#endif //DRAW_SERVICES_CPP
