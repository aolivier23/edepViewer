//File: State.cpp
//Brief: A State is a procedure for an evd::Window to interact with the user.  State::poll() is 
//       called every time evd::Window::Render() is called which means once per OpenGL frame.  
//       If a State returns a non-null std::unique_ptr<State> from poll, evd::Window will transition
//       to that new State.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//Local includes
#include "State.h"
#include "Disable.cpp"

//app includes
#include "app/Window.h"

std::unique_ptr<fsm::State> fsm::State::poll(const int width, const int height, const ImGuiIO& io, evd::Window& window)
{
  auto newState = Draw(width, height, io, window);
  if(newState) return newState;
  return doPoll(window);
}

std::unique_ptr<fsm::State> fsm::State::Draw(const int width, const int height, const ImGuiIO& io, evd::Window& window)
{
  ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x, 0.f), ImGuiCond_Always, ImVec2(1.0f, 1.0));
  ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, 20.f));
  ImGui::Begin("Control Bar", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
  if(ImGui::Button("Print")) //Print button always works
  {
    window.Print(width, height);
  }
  {
    detail::Disable disabled(ImGuiCol_Button, ImGuiCol_ButtonHovered, ImGuiCol_ButtonActive, ImGuiCol_Text);
    ImGui::SameLine();
    //Draw other buttons with disabled colors, but don't do anything in response to them
    int ids[] = {window.CurrentEvent().runID, window.CurrentEvent().eventID};
    ImGui::InputInt2("(Run, Event)", ids, ImGuiInputTextFlags_EnterReturnsTrue);
    ImGui::SameLine();

    ImGui::Button("Next");
    ImGui::SameLine();
    ImGui::Button("Reload");
    ImGui::SameLine();
    ImGui::Button("File");
    ImGui::SameLine();
    ImGui::ProgressBar(((float)window.EventCacheSize())/((float)window.MaxEventCacheSize()));
    if(ImGui::IsItemHovered())
    {
      ImGui::BeginTooltip();
      ImGui::Text((std::to_string(window.EventCacheSize())+" events cached out of "
                   +std::to_string(window.MaxEventCacheSize())+" cache size.").c_str());
      ImGui::EndTooltip();
    }

  }
  ImGui::End();

  window.Render(width, height, io);

  return nullptr; //Return a nullptr -> no transition
}
