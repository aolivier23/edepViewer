//File: Controller.h
//Brief: An evd::Controller is a state machine that connects input from the user to
//       an evd::Window for rendering.  This is the user's only point of interaction 
//       with evd::Window.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//Local includes
#include "Controller.h"
#include "Window.h"
#include "Source.h"

//states includes
#include "states/State.h"
#include "states/FirstEvent.h"

//c++ includes
#include <chrono>

evd::Controller::Controller(std::unique_ptr<YAML::Node>&& config, 
                            std::unique_ptr<src::Source>&& source): fWindow(new Window(std::move(config), std::move(source))), 
                                                                    fState(std::unique_ptr<fsm::State>(new fsm::FirstEvent()))
{
}

evd::Controller::Controller(const int argc, const char** argv): Controller(FindConfig(argc, argv), FindSource(argc, argv))
{
}

evd::Controller::~Controller()
{
}

void evd::Controller::Render(const int width, const int height, const ImGuiIO& ioState)
{
  auto newState = fState->poll(width, height, ioState, *fWindow);
  if(newState) fState = std::move(newState); //Implement State transition if requested
}

std::unique_ptr<src::Source> evd::Controller::FindSource(const int argc, const char** argv) const
{
  std::vector<std::string> rootFiles;
  for(int arg = 0; arg < argc; ++arg)
  {
    std::string name(argv[arg]);
    if(name.find(".root") != std::string::npos) rootFiles.push_back(name);
  }
                                                                                                                                                
  if(rootFiles.empty()) return std::unique_ptr<src::Source>(nullptr);
  return std::unique_ptr<src::Source>(new src::Source(rootFiles));
}
                                                                                                                                                
std::unique_ptr<YAML::Node> evd::Controller::FindConfig(const int argc, const char** argv) const
{
  std::unique_ptr<YAML::Node> doc(new YAML::Node());
  bool found = false;
  for(int arg = 0; arg < argc; ++arg)
  {
    std::string name(argv[arg]);
    if(name.find(".yaml") != std::string::npos) 
    { 
      std::ifstream file(name);
      doc.reset(new YAML::Node(YAML::Load(file)));
      if(!doc->IsNull()) found = true;
    }
  }
                                                                                                                                                
  //If not configuration file found, fall back to default installed with this package
  if(!found)
  {
    std::ifstream file(INSTALL_YAML_DIR "/default.yaml");
    doc.reset(new YAML::Node(YAML::Load(file)));
    if(doc->IsNull())
    {
      throw std::runtime_error("Failed to find a YAML configuration file named " INSTALL_YAML_DIR "/default.yaml in the current directory.\n");
    }
  }
  return doc;
}
