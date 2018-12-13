//File: Goto.h
//Brief: A Goto is a NonSequential State in which the EvdWindow is waiting for an opportunity to 
//       clear the cache of all pre-processed events and skip to a new position in the Source.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//Local includes
#include "Goto.h"

//app includes
#include "app/Window.h"

//TODO: Remove me
#include <iostream>

fsm::Goto::Goto(const int run, const int event): NonSequential(), fRun(run), fEvent(event)
{
}

void fsm::Goto::seekToEvent(evd::Window& window)
{
  std::cout << "Telling Window to ProcessEvent from Goto::seekToEvent()\n";
  window.ProcessEvent(fRun, fEvent);
}
