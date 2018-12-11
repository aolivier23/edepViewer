//File: TryLoadNextEvent.cpp
//Brief: A TryNextEvent State displays the previous event until the first event in the 
//       event cache is ready to be displayed.  When that event is ready, it updates all 
//       Scenes and causes a transition() to the Running State.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//Local includes
#include "TryLoadNextEvent.h"
#include "Running.h"

//app includes
#include "app/Window.h"

//The core ImGUI function definitions
#include "imgui.h"

//OpenGL functions get provided through this include
#include "glad/include/glad/glad.h"

fsm::TryLoadNextEvent::TryLoadNextEvent()
{
}

std::unique_ptr<fsm::State> fsm::TryLoadNextEvent::doPoll(evd::Window& window)
{
  if(window.NextEventStatus().wait_for(std::chrono::milliseconds(10)) == std::future_status::timeout)
  {
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    //Draw a window informing the user that the next event is loading now
    ImGui::Begin("Loading"); //TODO: Put this window off to the side, but keep it obvious
    ImGui::Text("Loading, next event and/or file...");
    ImGui::End();
    return nullptr;
  }

  //If the next event is ready, load it into the window and go to the Running state
  window.LoadNextEvent();
  return std::unique_ptr<State>(new Running());
}
