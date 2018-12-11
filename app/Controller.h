//File: Controller.h
//Brief: An evd::Controller is a state machine that connects input from the user to
//       an evd::Window for rendering.  This is the user's only point of interaction 
//       with evd::Window.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef EVD_CONTROLLER_H
#define EVD_CONTROLLER_H

//c++ includes
#include <memory>
#include <future>

namespace fsm
{
  class State;
}

namespace src
{
  class Source;
}

namespace YAML
{
  class Node;
}

class ImGuiIO;

namespace evd
{
  class Window;

  class Controller
  {
    public:
      Controller(std::unique_ptr<YAML::Node>&& config, std::unique_ptr<src::Source>&& source);
      Controller(const int argc, const char** argv);
      virtual ~Controller();

      void Render(const int width, const int height, const ImGuiIO& ioState); //Render one frame.

    private:
      std::unique_ptr<Window> fWindow; //The window rendering the user's data
      std::unique_ptr<fsm::State> fState; //The current user interaction mode

      //Helper functions to configure a Controller from the command line
      std::unique_ptr<src::Source> FindSource(const int argc, const char** argv) const;
      std::unique_ptr<YAML::Node> FindConfig(const int argc, const char** argv) const;
  };
}

#endif //EVD_CONTROLLER_H
