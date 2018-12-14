//File: GeoDrawer.cpp
//Brief: A GeoDrawerBase is a plugin that creates a SceneController, caches an appropriate SceneModel for each event, 
//       and applies that SceneModel to its' SceneController when requested.  The user should derive from the GeoDrawer 
//       class template which implements most of this behavior so that the user just has to request a particular 
//       vertex type and produce a SceneModel, typedefed model_t, for each event.  GeoDrawerBase defines the high-level 
//       interface that the application will control, and GeoDrawer<HANDLE> helps the user implement this interface.  
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef DRAW_GEOCONTROLLER_CPP
#define DRAW_GEOCONTROLLER_CPP

//Implementation of functionality of GeoDrawer
#include "plugins/Factory.cpp"
#include "plugins/drawing/Controller.cpp"
#include "plugins/drawing/Services.cpp"

//ROOT includes
#include "TGeoManager.h"

namespace draw
{
  using GeoControllerBase = detail::ControllerBase<const TGeoManager&, Services&>;

  template <class DRAWER>
  using GeoController = detail::Controller<DRAWER, const TGeoManager&, Services&>;
}

#define REGISTER_GEO(DERIVED) \
namespace \
{ \
  static plgn::Registrar<GeoControllerBase, GeoController<DERIVED>> DERIVED_geometry_reg(#DERIVED); \
}

#endif //DRAW_GEOCONTROLLER_CPP
