//File: EvdApp.cpp
//Brief: A Gtk::Application for handling input to an EvdWindow.  Expects 0 or more files on the command line.  An xml configuration 
//       file must be provided, and this application will search for it in the current directory if the user does not provide one 
//       on the command line.  Opens a dialog for choosing a file if the user does not provide a .root file on the command line.  
//       Quits with an error message on any other file extensions. 
//Author: Andrew Olivier aolivier@ur.rochester.edu

//Header
#include "gtk/EvdApp.h"

//local includes
#include "gtk/Source.h"

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
    std::vector<std::string> rootFiles;

    for(const auto& file: files)
    {
      //Look for an XML configuration file.  Must have a .xml extension.
      const auto name = file->get_path();
      if(file->get_basename().find(".xml") != std::string::npos) 
      {
        const auto status = config->LoadFile(name.c_str());
        if(status != tinyxml2::XML_SUCCESS) 
        {
          throw std::runtime_error("Got error "+std::to_string(status)+" when trying to load configuration file "+name+" with tinyxml2.\n");
        }
      }

      //The rest of these must be .root files, so create a TChain to read them all.
      else if(file->get_basename().find(".root") != std::string::npos) 
      {
        rootFiles.push_back(file->get_uri()); //I used to use name here, but uri seems to be name with additional protocol information for xrootd
      }
      else std::cerr << "Got file " << name << " that is neither an XML configuration file nor a .root file.\n";
    }

    //Create an EvdWindow that will be shown. 
    fWindow.reset(new EvdWindow());
    add_window(*fWindow);

    fWindow->reconfigure(std::move(config));
    std::cout << "Before constructing Source, rootFiles.size() is " << rootFiles.size() << "\n";
    fWindow->SetSource(std::unique_ptr<src::Source>(new src::Source(rootFiles)));   

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
      throw std::runtime_error("Failed to find an XML configuration file named default.xml in the current directory.\n");
    }

    //Create an EvdWindow that will be shown. 
    fWindow.reset(new EvdWindow());
    add_window(*fWindow);

    Gtk::FileChooserDialog chooser(*fWindow, "Choose an edep-sim file to view");
    //TODO: Parent window?
    //TODO: configure dialog to only show .root files and directories

    chooser.add_button("Open", Gtk::RESPONSE_OK);

    const int result = chooser.run();
    std::string input("");
    if(result == Gtk::RESPONSE_OK)
    {
      input = chooser.get_filename();
    }
    else throw std::runtime_error("Failed to get edepsim ROOT file for reading.\n");

    //Configure EvdWindow
    fWindow->reconfigure(std::move(config));
    fWindow->SetSource(std::unique_ptr<src::Source>(new src::Source(input)));

    fWindow->present(); 
  }
} 
