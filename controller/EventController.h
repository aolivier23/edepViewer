//File: EventController.h
//Brief: Abstract base class for event selection controller.  An EventController 
//       communicates with the source of event information used to populate the Model
//       and updates the Model and each View component when the event changes.  An
//       EventController is also a GUI component that provides the user's interface to 
//       event control.  
//Author: Andrew Olivier aolivier@ur.rochester.edu

//gtkmm includes
#include "gtkmm.h"

//c++ includes
#include <memory>
#include <string>

#ifndef CONT_EVENTCONTROLLER_H
#define CONT_EVENTCONTROLLER_H

class TFile;
class TTreeReader;

namespace view
{
  class Viewer;
}

namespace cont
{
  class EventController: public Gtk::Box
  {
    public:
      EventController(TFile& file, const std::string& treeName);
      virtual ~EventController() = default;

      virtual void AddViewer(view::Viewer* viewer);

    protected:
      UpdateEvent() const; //Call this function after updating the event
      UpdateFile() const; //Call this function after updating the geometry.  Also calls UpdateEvent().

    private:
      //TODO: Real Model class instead of a TTree
      TTreeReader fReader;
      std::vector<view::Viewer*> fViewers; //Viewers to update on event change
  };
}

#endif //CONT_EVENTCONTROLLER_H

