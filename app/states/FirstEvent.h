//File: FirstEvent.h
//Brief: When an evd::Window is first created, it doesn't yet have a previous event to display.  
//       It turns out that drawing the control bar in this state is very hard with my current 
//       application model.  So, evd::Controller is created in the FirstEvent State which does 
//       not render the control bar and transitions to the Running() state as soon as possible.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef FSM_FIRSTEVENT_H
#define FSM_FIRSTEVENT_H

//Local includes
#include "State.h"

namespace fsm
{
  class FirstEvent: public State
  {
    public:
      FirstEvent();
      virtual ~FirstEvent() = default;

    protected:
      virtual std::unique_ptr<State> doPoll(evd::Window& window) override;
      virtual std::unique_ptr<State> Draw(const int width, const int height, const ImGuiIO& io, evd::Window& window) override;
  };
}

#endif //FSM_FIRSTEVENT_H
