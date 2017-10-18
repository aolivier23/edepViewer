//File: EvdWindow.h
//Brief: The main Gtk::ApplicationWindow that will show the Viewers given to it.  
//       Composes Viewers into a tab GUI and sets up controllers to function as the 
//       top-level controller.
//       The model exists elsewhere.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//gtkmm include
#include <gtkmm.h>

//c++ includes
#include <memory>

#ifndef VIEW_EVDWINDOW_H
#define VIEW_EVDWINDOW_H

namespace view
{
  class Viewer
}

namespace cont
{
  class EventController;
  
  class EvdWindow: public Gtk::ApplicationWindow
  {
    public:
      EvdWindow(std::shared_ptr<EventController> evtCont);
      virtual ~EvdWindow() = default;

      virtual void AddViewer(view::Viewer& viewer);

    protected:
      Gtk::Box fMainBox; //The box that contains all of the other widgets.  Will be aligned vertically so that 
                         //fEventCont is a small bar above fViewers.
      Gtk::Notebook fViewers; //Manages the Viewers as tabs.  Internally keeps the list of Viewers.
      std::shared_ptr<EventController> fEventCont; //GUI that responds to user requests for event updates and tells the Viewers 
                                  //in this window to update themselves 
      //TODO: Selection controller
      //TODO: Configuration tab?

      virtual void on_realize() override; //The event display application will be restarted for each new event.  So, update all viewers 
                                          //at this point. 
  };
}

#endif //VIEW_EVDWINDOW_H
