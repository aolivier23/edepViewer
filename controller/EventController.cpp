//File: EventController.cpp
//Brief: Interface for a GUI that controls what event is retrieved by Viewers.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//view includes
#include "view/gtk/Viewer.h"

//ROOT includes
#include "TFile.h"
#include "TTreeReader.h"

namespace cont
{
  EventController::EventController(TFile& file, const std::string& treeName): fReader(treeName.c_str(), file)
  {
  }

  void EventController::AddViewer(view::Viewer* viewer)
  {
    fViewers.push_back(viewer);
  }

  //Call this after updating the TTreeReader
  void EventController::UpdateEvent()
  {
    for(auto viewer: fViewers) 
    {
      viewer->ReadEvent();
    }
  }

  void EventController::UpdateFile()
  {
    for(auto viewer: fViewers)
    {
      viewer->ReadGeo();
      viewer->ReadEvent();
    }
  }
}

