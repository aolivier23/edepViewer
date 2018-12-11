//File: FirstEvent.cpp
//Brief: When an evd::Window is first created, it doesn't yet have a previous event to display.  
//       It turns out that drawing the control bar in this state is very hard with my current 
//       application model.  So, evd::Controller is created in the FirstEvent State which does 
//       not render the control bar and transitions to the Running() state as soon as possible.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//Local includes
#include "FirstEvent.h"
#include "TryLoadNextEvent.h"

//The core ImGUI function definitions
#include "imgui.h"

//OpenGL functions get provided through this include
#include "glad/include/glad/glad.h"

fsm::FirstEvent::FirstEvent(): State()
{
}

std::unique_ptr<fsm::State> fsm::FirstEvent::doPoll(const bool allEventsReady, evd::Window& /*window*/)
{
  if(allEventsReady) return std::unique_ptr<State>(new TryLoadNextEvent());
  return nullptr;
}

std::unique_ptr<fsm::State> fsm::FirstEvent::Draw(const int /*width*/, const int /*height*/, const ImGuiIO& /*io*/, evd::Window& /*window*/)
{
  glClearColor(0.0, 0.0, 0.0, 1.0);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

  //Draw a window informing the user that the next event is loading now
  ImGui::Begin("Loading"); //TODO: Put this window off to the side, but keep it obvious
  ImGui::Text("Loading first event and file...");
  ImGui::End();

  return nullptr; //Nothing to draw yet
}
