//File: EventController.cpp
//Brief: An EventController is a plugin that creates a SceneController, caches an appropriate SceneModel for each event, 
//       and applies that SceneModel to its' SceneController when requested.  The user should derive from the GeoDrawer 
//       class template which implements most of this behavior so that the user just has to request a particular 
//       vertex type and produce a SceneModel, typedefed model_t, for each event.  GeoDrawerBase defines the high-level 
//       interface that the application will control, and GeoDrawer<HANDLE> helps the user implement this interface.  
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef DRAW_EVENTCONTROLLER_CPP
#define DRAW_EVENTCONTROLLER_CPP

//Implementation of functionality of GeoDrawer
#include "plugins/Factory.cpp"
#include "plugins/drawing/Controller.cpp"
#include "plugins/drawing/Services.cpp"

namespace draw
{
  using EventControllerBase = detail::ControllerBase<const TG4Event&, Services&>;

  template <class DRAWER>
  using EventController = detail::Controller<DRAWER, const TG4Event&, Services&>;
}

#define REGISTER_EVENT(DERIVED) \
namespace \
{ \
  static plgn::Registrar<EventControllerBase, EventController<DERIVED>> DERIVED_event_reg(#DERIVED); \
}

#endif //DRAW_EVENTCONTROLLER_CPP
