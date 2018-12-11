//File: TryLoadNextEvent.h
//Brief: A TryNextEvent State displays the previous event until the first event in the 
//       event cache is ready to be displayed.  When that event is ready, it updates all 
//       Scenes and causes a transition() to the Running State.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef FSM_TRYLOADNEXTEVENT_H
#define FSM_TRYLOADNEXTEVENT_H

//Local includes
#include "State.h"

namespace fsm
{
  class TryLoadNextEvent: public State
  {
    public:
      TryLoadNextEvent();
      virtual ~TryLoadNextEvent() = default;

    protected:
      virtual std::unique_ptr<State> doPoll(const bool allEventsReady, evd::Window& window) override;
  };
}

#endif //FSM_TRYLOADNEXTEVENT_H
