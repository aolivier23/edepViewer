//File: EvdWindow.cpp
//Brief: The main event display window.  Manages a list of Viewers and the global controllers that 
//       interact with those Viewers.  Additional global features, like overall window configuration or 
//       a message center, should be added here.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//cont includes
#include "EvdWindow.h"
#include "EventController.h"

namespace cont
{
  EvdWindow::EvdWindow(std::shared_ptr<EventController> evtController): fEventCont(evtController), fViewers(), fMainBox(Gtk::ORIENTATION_VERTICAL)
  {
    add(fMainBox);
    fMainBox.pack_start(*fEventCont, Gtk::PACK_SHRINK);
    fMainBox.pack_end(fViewers);
 
    show_all_children();    
  }

  void EvdWindow::AddViewer(view::Viewer& viewer, const std::string& name)
  {
    view.ReadGeo();
    fViewers.append_page(viewer,  name);
  }

  void on_realize()
  {
    auto viewers = fViewers.get_children();
    for(auto widget: viewers)
    {
      auto viewer = (view::Viewer*)(widget);
      viewer->ReadEvent();
      //TODO: Deal with geometry updates.  Probably need a custom Gtk::Application for this.
    }
  }
}
