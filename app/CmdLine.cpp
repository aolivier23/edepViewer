//File: CmdLine.cpp
//Brief: Global functions to find the names of configuration files and edep-sim input files from the command line.  Call from 
//       main() and send results to an EvdWindow.  Tools to let main() replace EvdApp from old gtkmm design.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//Configuration parser includes
#include "yaml-cpp/yaml.h"

//local includes
#include "app/Source.h"

namespace cmd
{
  std::unique_ptr<src::Source> FindSource(const int argc, const char** argv)
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

  //TODO: This function needs to change when I switch to yaml
  std::unique_ptr<YAML::Node> FindConfig(const int argc, const char** argv)
  {
    std::unique_ptr<YAML::Node> doc(new YAML::Node());
    bool found = false;
    for(int arg = 0; arg < argc; ++arg)
    {
      std::string name(argv[arg]);
      if(name.find(".yaml") != std::string::npos) 
      { 
        doc.reset(new YAML::Node(YAML::Load(name.c_str())));
        if(!doc->IsNull()) found = true;
      }
    }

    //If not configuration file found, fall back to default installed with this package
    if(!found)
    {
      doc.reset(new YAML::Node(YAML::Load(INSTALL_XML_DIR "/default.yaml")));
      if(doc->IsNull())
      {
        throw std::runtime_error("Failed to find a YAML configuration file named " INSTALL_XML_DIR "/default.yaml in the current directory.\n");
      }
    }
    return doc;
  }
}
