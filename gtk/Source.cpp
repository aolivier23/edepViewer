//File: Source.cpp
//Brief: A Source for this edepsim display provides access to a TG4Event and a TGeoManager.  It knows how to go to the 
//       next event as well as an arbitrary event offset.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//Header
#include "gtk/Source.h"

//ROOT includes
#include "TFile.h"

namespace src
{
  Source::Source(const std::vector<std::string>& files): fTree("EDepSimEvents"), fReader(&fTree), fEvent(fReader, "Event")
  {
    for(const auto& file: files) fTree.Add(file.c_str());
    fReader.Next();
  }

  Source::Source(const std::string& file): fTree("EDepSimEvents"), fReader(&fTree), fEvent(fReader, "Event")
  {
    fTree.Add(file.c_str());
    fReader.Next();
  }

  const TG4Event& Source::Event()
  {
    return *fEvent;
  }

  TGeoManager* Source::Geo()
  {
    auto file = fTree.GetFile();
    auto geo = (TGeoManager*)file->Get("EDepSimGeometry");
    if(geo == nullptr) throw std::runtime_error("Failed to get geometry object from file named "+std::string(file->GetName())+"\n");
    return geo;
  }

  void Source::Next()
  {
    if(!(fReader.Next())) fReader.Restart();
  }

  bool Source::GoTo(const size_t evt)
  {
    return fReader.SetEntry(evt) == TTreeReader::kEntryValid;
  }

  const size_t Source::Entry()
  {
    return fReader.GetCurrentEntry();
  }

  const std::string Source::GetFile()
  {
    return std::string(fTree.GetFile()->GetName());
  }
}
