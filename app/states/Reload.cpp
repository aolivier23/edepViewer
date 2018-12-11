//File: Reload.h
//Brief: A Reload is a NonSequential State that clears the cache of all events and
//       Starts refilling it from the same position.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//Local includes
#include "Reload.h"

fsm::Reload::Reload(): NonSequential()
{
}

void fsm::Reload::seekToEvent(evd::Window& window)
{
}
