//File: State.cpp
//Brief: A State is a procedure for an evd::Window to interact with the user.  State::poll() is 
//       called every time evd::Window::Render() is called which means once per OpenGL frame.  
//       If a State returns a non-null std::unique_ptr<State> from poll, evd::Window will transition
//       to that new State.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//Local includes
#include "State.h"

//app includes
#include "app/Window.h"

//The core ImGUI function definitions
#include "imgui.h"


namespace
{
  //Helper structs to count number of arguments at compile-time
  template <class T, class ...ARGS>
  struct Count
  {
    static constexpr size_t argc = Count<ARGS...>::argc;
  };
 
  //Special case of Count: only 1 argument.  Ends recursion.
  template <class T>
  struct Count<T>
  {
    static constexpr size_t argc = 1;
  };

  //RAII-controlled stack of ImGui color settings.  
  //Sets all passed colors to "disabled" color at construction
  //and sets the same number of colors back to original color
  //at destruction.
  struct Disable
  {
    template <class ...ARGS>
    Disable(ARGS... args): fNArgs(Count<ARGS...>::argc)
    {
      using expander = int[];
      (void)(expander {0, (doDisable(args), 0)...});
    }

    ~Disable()
    {
      ImGui::PopStyleColor(fNArgs);
    }

    private:
      const size_t fNArgs;

      void doDisable(const ImGuiCol color)
      {
        const auto old = ImGui::GetStyleColorVec4(color);
        ImGui::PushStyleColor(color, ImVec4(old.x, old.y, old.z, fDisabledAlpha*old.w));
      }

      static constexpr float fDisabledAlpha = 0.25; //Ratio of diabled transparency to regular transparency
  };
}

std::unique_ptr<fsm::State> fsm::State::poll(const int width, const int height, const ImGuiIO& io, const bool allEventsReady, evd::Window& window)
{
  auto newState = Draw(width, height, io, window);
  if(newState) return newState;
  return doPoll(allEventsReady, window);
}

std::unique_ptr<fsm::State> fsm::State::Draw(const int width, const int height, const ImGuiIO& io, evd::Window& window)
{
  {
    ::Disable disabled(ImGuiCol_Button, ImGuiCol_ButtonHovered, ImGuiCol_ButtonActive, ImGuiCol_Text);
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x, 0.f), ImGuiCond_Always, ImVec2(1.0f, 1.0));
    ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, 20.f));
    ImGui::Begin("Control Bar", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
  }
  if(ImGui::Button("Print")) //Print button always works
  {
    window.Print(width, height);
  }
  ::Disable disabled(ImGuiCol_Button, ImGuiCol_ButtonHovered, ImGuiCol_ButtonActive, ImGuiCol_Text);
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
  //TODO: Status of event pre-loading?

  window.Render(width, height, io);

  return nullptr; //Return a nullptr -> no transition
}
