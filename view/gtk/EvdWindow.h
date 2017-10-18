//File: EvdWindow.h
//Brief: The main Gtk::ApplicationWindow that will show the Viewers given to it.  
//       The model exists elsewhere.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//gtkmm include
#include <gtkmm.h>

#ifndef VIEW_EVDWINDOW_H
#define VIEW_EVDWINDOW_H

namespace cont
{
  class EventController;
}

namespace view
{
  class Viewer;
  
  class EvdWindow: public Gtk::ApplicationWindow
  {
    public:
      EvdWindow(cont::EventController& evtCont);
      virtual ~EvdWindow() = default;

      virtual void AddViewer(view::Viewer& viewer);

    protected:
      Gtk::Notebook fViewers; //Manages the Viewers as tabs
      cont::EventController fEventCont; //GUI that responds to user requests for event updates and tells the Viewers 
                                        //in this window to update themselves 
      //TODO: Selection controller
      //TODO: Configuration tab?
  };
}

#endif //VIEW_EVDWINDOW_H
