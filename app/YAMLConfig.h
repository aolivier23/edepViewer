//File: YAMLConfig.h
//Brief: Interprets a YAML::Node (from yaml-cpp) as a tree of other YAML::Nodes.  Node names cannot be changed, but mappings' values can, and 
//       Nodes can be added or removed.  Can save the final YAML configuration to a file.  
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef GUI_YAMLCONFIG_H
#define GUI_YAMLCONFIG_H

//yaml-cpp includes
#include "yaml-cpp/yaml.h"

namespace gui
{
  class YAMLConfig
  {
    public:
      YAMLConfig(YAML::Node& node): fConfig(node), fShowGUI(false) {}
      virtual ~YAMLConfig() = default;
  
      //Render via ImGUI.  Returns true if configuration has changed.
      bool Render();

      //Access to configuration interface
      const YAML::Node& GetConfig() const { return fConfig; }

      //Make visible.  This object hides itself when done.
      void Show() { fShowGUI = true; }
  
    protected:
      YAML::Node fConfig; //The current application configuration
      bool fShowGUI;

    private:
      void Render(YAML::Node node); //TODO: I really want to take this by reference, but that doesn't seem to be an option in the YAML::Node API!
  };
}

#endif //GUI_YAMLCONFIG_H
