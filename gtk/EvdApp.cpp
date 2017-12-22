//File: EvdApp.cpp
//Brief: A Gtk::Application for handling input to an EvdWindow.  Expects 0 or more files on the command line.  An xml configuration 
//       file must be provided, and this application will search for it in the current directory if the user does not provide one 
//       on the command line.  Opens a dialog for choosing a file if the user does not provide a .root file on the command line.  
//       Quits with an error message on any other file extensions. 
//Author: Andrew Olivier aolivier@ur.rochester.edu

//Header
#include "gtk/EvdApp.h"

//ROOT includes
#include "TChain.h"

namespace mygl
{
  Glib::RefPtr<EvdApp> EvdApp::create()
  {
    return Glib::RefPtr<EvdApp>(new EvdApp());
  }

  void EvdApp::on_open(const std::vector<Glib::RefPtr<Gio::File>>& files, const Glib::ustring& hints)
  {
    TChain input;
    tinyxml2::XMLDocument config;    

    for(const auto& file: files)
    {
      //Look for an XML configuration file.  Must have a .xml extension.
      const auto name = file->get_uri();
      if(file->get_basename().find(".xml") != std::string::npos) 
      {
        const auto status = config.LoadFile(name.c_str());
        if(status != tinyxml2::XML_SUCCESS) 
        {
          std::cerr << "Got error " << status << " when trying to load configuration file " << name << " with tinyxml2.\n";
        }
      }

      //The rest of these must be .root files, so create a TChain to read them all.
      else if(file->get_basename().find(".root") != std::string::npos) input.Add(name.c_str());
      else std::cerr << "Got file " << name << " that is neither an XML configuration file nor a .root file.\n";
    }

    //If an XML file was not provided, look for one in the current directory.
    //TODO: Look for a configuration file at some standard path instead?
    const auto status = config.LoadFile("default.xml");
    if(status != tinyxml2::XML_SUCCESS) 
    {
      std::cerr << "Failed to find an XML configuration file named default.xml in the current directory.\n";
    }

    //If no .root files were provided, open a dialog to choose an edepsim file.
    Gtk::FileChooserDialog chooser("Choose an edep-sim file to view");
    //TODO: configure dialog to only show .root files and directories

    chooser.add_button("Open", Gtk::RESPONSE_OK);

    const int result = chooser.run();

    if(result == Gtk::RESPONSE_OK)
    {
      input.Add(chooser.get_filename().c_str());
    }

    for(const auto& window: get_windows())
    {
      const auto& evd = *dynamic_cast<EvdWindow*>(window);
      evd.reconfigure(config);
      evd.set_file(input);
    }
  }

  void EvdApp::on_activate()
  {
    //Create an EvdWindow that will be shown. 
    fWindow.reset(new EvdWindow());
    add_window(*fWindow);
    fWindow->present(); 
  }
} 
