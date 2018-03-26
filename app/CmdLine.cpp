//File: CmdLine.cpp
//Brief: Global functions to find the names of configuration files and edep-sim input files from the command line.  Call from 
//       main() and send results to an EvdWindow.  Tools to let main() replace EvdApp from old gtkmm design.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//Configuration parser includes
#include <tinyxml2.h>

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
  std::unique_ptr<tinyxml2::XMLDocument> FindConfig(const int argc, const char** argv)
  {
    std::unique_ptr<tinyxml2::XMLDocument> doc(new tinyxml2::XMLDocument());
    bool found = false;
    for(int arg = 0; arg < argc; ++arg)
    {
      std::string name(argv[arg]);
      if(name.find(".xml") != std::string::npos) 
      {
        if(doc->LoadFile(name.c_str()) == tinyxml2::XML_SUCCESS) found = true;
      }
    }

    //If not configuration file found, fall back to default installed with this package
    if(!found)
    {
      const auto status = doc->LoadFile(INSTALL_XML_DIR "/default.xml");
      if(status != tinyxml2::XML_SUCCESS)
      {
        throw std::runtime_error("Failed to find an XML configuration file named " INSTALL_XML_DIR "/default.xml in the current directory.\n");
      }
    }
    return doc;
  }
}
