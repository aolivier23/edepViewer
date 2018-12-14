//File: Goto.h
//Brief: A Goto is a NonSequential State in which the EvdWindow is waiting for an opportunity to 
//       clear the cache of all pre-processed events and skip to a new position in the Source.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef FSM_GOTO_H
#define FSM_GOTO_H

//Local includes
#include "NonSequential.h"

namespace fsm
{
  class Goto: public NonSequential
  {
    public: 
      Goto(const int run, const int evt);
      virtual ~Goto() = default;

    protected:
      virtual void seekToEvent(evd::Window& evd) override;

      const int fRun; //run the user wants to see next
      const int fEvent; //event the user wants to see next
  };
}

#endif //FSM_GOTO_H
