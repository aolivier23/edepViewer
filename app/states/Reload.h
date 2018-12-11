//File: Reload.h
//Brief: A Reload is a NonSequential State that clears the cache of all events and
//       Starts refilling it from the same position.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef FSM_RELOAD_H
#define FSM_RELOAD_H

//Local includes
#include "NonSequential.h"

namespace fsm
{
  class Reload: public NonSequential
  {
    public:
      Reload();
      virtual ~Reload() = default;

    protected:
      virtual void seekToEvent(evd::Window& window) override; 
  };
}

#endif //FSM_RELOAD_H
