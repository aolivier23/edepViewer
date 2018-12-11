//File: Running.cpp
//Brief: Running is a State in which the EvdWindow lets the user interact with a GUI to trigger transitions
//       to other States.  While Running, try to keep the cache of future events full.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//Local includes
#include "Running.h"
#include "Goto.h"
#include "TryLoadNextEvent.h"
#include "Reload.h"
#include "ChooseFile.cpp"
#include "NewFile.h"

//app includes
#include "app/Window.h"

namespace fsm
{
  Running::Running(): State()
  {
  }

  std::unique_ptr<State> Running::Draw(const int width, const int height, const ImGuiIO& io, evd::Window& window)
  {
    std::unique_ptr<State> transition(nullptr);
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x, 0.f), ImGuiCond_Always, ImVec2(1.0f, 1.0));
    ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, 20.f));
    ImGui::Begin("Control Bar", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    if(ImGui::Button("Print"))
    {
      window.Print(width, height);
    } 
    ImGui::SameLine();

    int ids[] = {window.CurrentEvent().runID, window.CurrentEvent().eventID};
    if(ImGui::InputInt2("(Run, Event)", ids, ImGuiInputTextFlags_EnterReturnsTrue)) transition = std::unique_ptr<State>(new Goto(ids[0], ids[1]));
    ImGui::SameLine();

    if(ImGui::Button("Next")) transition = std::unique_ptr<State>(new TryLoadNextEvent());
    ImGui::SameLine();
    if(ImGui::Button("Reload")) transition = std::unique_ptr<State>(new Reload());
    ImGui::SameLine();
    if(ImGui::Button("File")) transition = std::unique_ptr<State>(new ChooseFile<NewFile>("*.root"));
    //TODO: Status of event processing?
    ImGui::End();

    window.Render(width, height, io);

    return transition;
  }

  std::unique_ptr<State> Running::doPoll(evd::Window& window)
  {
    if(!window.LastEventStatus().valid() && window.EventCacheSize() < window.MaxEventCacheSize())
    {
      window.ProcessEvent(false);
    }
    return std::unique_ptr<State>(nullptr); //All transitions Running triggers happen in RenderControlBar()
  }
}
