//File: Goto.h
//Brief: A Goto is a NonSequential State in which the EvdWindow is waiting for an opportunity to 
//       clear the cache of all pre-processed events and skip to a new position in the Source.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//Local includes
#include "Goto.h"

//app includes
#include "app/Window.h"

fsm::Goto::Goto(const int run, const int event): NonSequential(), fRun(run), fEvent(event)
{
}

void fsm::Goto::seekToEvent(evd::Window& window)
{
  window.ProcessEvent(fRun, fEvent);
}
