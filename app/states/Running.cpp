//File: Running.cpp
//Brief: Running is a State in which the EvdWindow lets the user interact with a GUI to trigger transitions
//       to other States.  While Running, try to keep the cache of future events full.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//Local includes
#include "Running.h"
#include "Goto.h"
#include "TryLoadNextEvent.h"
#include "ChooseFile.cpp"
#include "NewFile.h"
#include "Disable.cpp"

//app includes
#include "app/Window.h"

namespace fsm
{
  Running::Running(const bool lastEvent): State(), fLastEvent(lastEvent)
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

    if(fLastEvent) //Disable Next event button if this is the last event in this Source
    {
      detail::Disable disabled(ImGuiCol_Button, ImGuiCol_ButtonHovered, ImGuiCol_ButtonActive);
      ImGui::Button("Next");
      if(ImGui::IsItemHovered()) 
      {
        ImGui::BeginTooltip();
        ImGui::Text("You are viewing the last event in the current Source.  To see more events, go to an earlier event or choose a new file.");
        ImGui::EndTooltip();
      }
    }
    else if(ImGui::Button("Next"))
    {
      if(window.EventCacheSize() < 1) window.ProcessEvent(false); //Make sure there's something in the event cache
      transition = std::unique_ptr<State>(new TryLoadNextEvent());
    }
    ImGui::SameLine();
    if(ImGui::Button("Reload")) transition = std::unique_ptr<State>(new Goto(window.CurrentEvent().runID, window.CurrentEvent().eventID));
    ImGui::SameLine();
    if(ImGui::Button("File")) transition = std::unique_ptr<State>(new ChooseFile<NewFile>(".root"));
    //TODO: Status of event processing?
    ImGui::End();

    window.Render(width, height, io);

    return transition;
  }

  std::unique_ptr<State> Running::doPoll(evd::Window& window)
  {
    if(!fLastEvent && ((window.EventCacheSize() == 0) || !window.LastEventStatus().valid()) && (window.EventCacheSize() < window.MaxEventCacheSize()))
    {
      window.ProcessEvent(false);
    }
    return std::unique_ptr<State>(nullptr); //All transitions Running triggers happen in RenderControlBar()
  }
}
