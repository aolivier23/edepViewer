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
//       TryLoadNextEvent State to wait on the event it just started processing. 
//Author: Andrew Olivier aolivier@ur.rochester.edu

//Local includes
#include "NonSequential.h"
#include "TryLoadNextEvent.h"

//app includes
#include "app/Window.h"

std::unique_ptr<fsm::State> fsm::NonSequential::doPoll(evd::Window& window)
{
  if((window.EventCacheSize() == 0) || (window.LastEventStatus().wait_for(std::chrono::milliseconds(10)) == std::future_status::ready))
  {
    window.ClearCache();
    seekToEvent(window);
    //window.ProcessEvent(true); //TODO: ProcessEvent() also makes its own changes to the source.  Derived classes should call 
                               //      ProcessEvent() instead of NonSequential.
    return std::unique_ptr<State>(new TryLoadNextEvent());
  }

  //Loading window
  //TODO: Just clear background in one place
  glClearColor(0.0, 0.0, 0.0, 1.0);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

  //Draw a window informing the user that the next event is loading now
  ImGui::Begin("Loading"); //TODO: Put this window off to the side, but keep it obvious
  ImGui::Text("Waiting for all events to finish processing before non-sequential access...");
  ImGui::End();

  return nullptr;
}
