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
  EvdApp::EvdApp(): Gtk::Application("Display.For.edepsim", Gio::APPLICATION_HANDLES_OPEN)
  {
  }

  Glib::RefPtr<EvdApp> EvdApp::create()
  {
    return Glib::RefPtr<EvdApp>(new EvdApp());
  }

  //Create an EvdWindow when files are provided on the command line.
  void EvdApp::on_open(const std::vector<Glib::RefPtr<Gio::File>>& files, const Glib::ustring& hints)
  {
    //TChain input;
    //TODO: Source class that knows how to handle multiple files
    std::string input("");
    std::unique_ptr<tinyxml2::XMLDocument> config(new tinyxml2::XMLDocument());   

    for(const auto& file: files)
    {
      //Look for an XML configuration file.  Must have a .xml extension.
      const auto name = file->get_path();
      if(file->get_basename().find(".xml") != std::string::npos) 
      {
        const auto status = config->LoadFile(name.c_str());
        if(status != tinyxml2::XML_SUCCESS) 
        {
          std::cerr << "Got error " << status << " when trying to load configuration file " << name << " with tinyxml2.\n";
        }
      }

      //The rest of these must be .root files, so create a TChain to read them all.
      else if(file->get_basename().find(".root") != std::string::npos) 
      {
        input = name; 
      }
      else std::cerr << "Got file " << name << " that is neither an XML configuration file nor a .root file.\n";
    }

    for(const auto& window: get_windows())
    {
      auto& evd = *dynamic_cast<EvdWindow*>(window);
      evd.reconfigure(std::move(config));
      evd.SetFile(input);
    }

    //Create an EvdWindow that will be shown. 
    fWindow.reset(new EvdWindow());
    add_window(*fWindow);

    fWindow->reconfigure(std::move(config));
    fWindow->SetFile(input);   

    fWindow->present();
  }

  //Create an EvdWindow when files are not provided on the command line
  void EvdApp::on_activate()
  {
    //Look for a default configuration file.
    //TODO: Look in some specific path?
    //If an XML file was not provided, look for one in the current directory.
    //TODO: Look for a configuration file at some standard path instead?
    std::unique_ptr<tinyxml2::XMLDocument> config(new tinyxml2::XMLDocument());
    const auto status = config->LoadFile("default.xml");
    if(status != tinyxml2::XML_SUCCESS)
    {
      std::cerr << "Failed to find an XML configuration file named default.xml in the current directory.\n";
      //TODO: Throw exception
    }

    Gtk::FileChooserDialog chooser("Choose an edep-sim file to view");
    //TODO: configure dialog to only show .root files and directories
    //TODO: Let Gtk automatically manage chooser

    chooser.add_button("Open", Gtk::RESPONSE_OK);

    const int result = chooser.run();
    std::string input("");
    if(result == Gtk::RESPONSE_OK)
    {
      input = chooser.get_filename();
    }
    //TODO: else throw exception?

    //Create an EvdWindow that will be shown. 
    fWindow.reset(new EvdWindow());
    add_window(*fWindow);

    fWindow->reconfigure(std::move(config));
    fWindow->SetFile(input);

    fWindow->present(); 
  }
} 
