//File: Disable.cpp
//Brief: A tool to set ImGui colors to be more transparent reflecting a disabled state.  
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef FSM_DETAIL_DISABLE_CPP
#define FSM_DETAIL_DISABLE_CPP

//The core ImGUI function definitions
#include "imgui.h"

namespace fsm
{
  namespace detail
  {
    //Helper structs to count number of arguments at compile-time
    template <class T, class ...ARGS>
    struct Count
    {
      static constexpr size_t argc = Count<ARGS...>::argc+1;
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
        for(size_t arg = 0; arg < fNArgs; ++arg) 
        {
          ImGui::PopStyleColor(); //fNArgs);
        }
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
}

#endif //FSM_DETAIL_DISABLE_CPP 
