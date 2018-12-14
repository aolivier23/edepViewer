//File: Running.h
//Brief: Running is a State in which the EvdWindow lets the user interact with a GUI to trigger transitions
//       to other States.  While Running, try to keep the cache of future events full.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef FSM_RUNNING_H
#define FSM_RUNNING_H

//Local includes
#include "State.h"

namespace fsm
{
  class Running: public State
  {
    public:
      Running(const bool lastEvent = false);
      virtual ~Running() = default;

    protected:
      virtual std::unique_ptr<State> doPoll(evd::Window& window) override;
      virtual std::unique_ptr<State> Draw(const int width, const int height, const ImGuiIO& io, evd::Window& window) override;

    private:
      const bool fLastEvent; //If this is the last event in the current file, ignore the Next button
  };
}

#endif //FSM_RUNNING_H
