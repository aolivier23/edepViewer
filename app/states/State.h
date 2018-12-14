//File: State.h
//Brief: A State is a procedure for an EvdWindow to interact with the user.  State::poll() is 
//       called every time EvdWindow::Render() is called which means once per OpenGL frame.  
//       If a State returns a non-null std::unique_ptr<State> from poll, EvdWindow will transition
//       to that new State.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef FSM_STATE_H
#define FSM_STATE_H

//c++ includes
#include <memory>

namespace evd
{
  class Window;
}

class ImGuiIO;

namespace fsm
{
  class State
  {
    public:
      //TODO: Giving the State constructor an evd::Window would make the creation of a State much more expressive as a transition
      State() = default;
      virtual ~State() = default;

      //Public interface with private implementation
      std::unique_ptr<State> poll(const int width, const int height, const ImGuiIO& io, evd::Window& window);

    protected:
      //Derived classes must implement this function.  They might also want to cache some state 
      //for using it upon creation.  If a non-null unique_ptr is returned, return that State as 
      //the new State for evd::Window to transition to.
      virtual std::unique_ptr<State> doPoll(evd::Window& window) = 0;

      //Draws a disabled control bar by default.  Override to react to control bar.  Returning a non-null 
      //unique_ptr prevents doPoll() from being called and returns that unique_ptr to evd::Window for a 
      //transition.
      virtual std::unique_ptr<State> Draw(const int width, const int height, const ImGuiIO& io, evd::Window& window);
  };
}

#endif //FSM_STATE_H
