//File: NonSequential.cpp
//Brief: A NonSequential is a kind of State that is waiting to implement a user 
//       request that cannot proceed while event processing is happening.  
//       Because the Running state is constantly starting threads to process 
//       new events, this kind of State might have to wait for many poll()s
//       before it can trigger a transition to a new State.  It doesn't 
//       make sense to cache additional requests while waiting, so draw 
//       an inactive control bar and a popup window to let the user know 
//       that we are waiting for event processing to stop.
//
//       A NonSequential State waits for all events to be finished while not 
//       starting any more, clears the event cache, starts processing of the 
//       event the user wants to access, and causes a transition to the 
//       TryLoadEvent State to wait on the event it just started processing. 
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef FSM_NONSEQUENTIAL_CPP
#define FSM_NONSEQUENTIAL_CPP

//Local includes
#include "State.h"

namespace fsm
{
  class NonSequential: public State
  {
    public:
      NonSequential(): State() {}
      virtual ~NonSequential() = default;

    protected:
      virtual std::unique_ptr<State> doPoll(evd::Window& window) override final;

      //NonSequential States just want to wait until they are ready
      //to perform some action that culminates in a transition().  
      //So, NonSequential does some common rendering for them, and 
      //derived classes must implement the dispatch() function and
      //return the State they're causing the EvdWindow to transition to.
      virtual void seekToEvent(evd::Window& window) = 0;
  };
}

#endif //FSM_NONSEQUENTIAL_CPP
