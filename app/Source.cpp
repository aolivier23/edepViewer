//File: Source.cpp
//Brief: A Source for this edepsim display provides access to a TG4Event and a TGeoManager.  It knows how to go to the 
//       next event as well as an arbitrary event offset.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//Header
#include "app/Source.h"

namespace src
{
  Source::Source(const std::vector<std::string>& files): fFileList(files), fFilePos(fFileList.begin()), fFile(),
                                                         fReader(), fEvent(fReader, "Event")
  {
    if(fFilePos == fFileList.end())
    {
      throw std::runtime_error("No files in Source.\n");
    }

    fFile.reset(TFile::Open((fFilePos)->c_str()));
    fReader.SetTree("EDepSimEvents", fFile.get());
    //TODO: Make a copy of this tree that passes some cuts instead?  Requires that those cuts come from user input somehow.  
    //      Should probably do this in a separate file so EvdWindow can parse the options file for cuts to apply.
    if(fReader.GetTree()->BuildIndex("RunId", "EventId") <= 0)
    {
      std::cerr << "In Source constructor, failed to build TTreeIndex on branches RunId and EventId.\n";
    }

    auto geo = (TGeoManager*)fFile->Get("EDepSimGeometry");
    if(geo == nullptr) throw std::runtime_error("Failed to get geometry object from file named "+std::string(fFile->GetName())+"\n");
    fGeo = geo;
    //fReader.Next();
  }

  Source::Source(const std::string& file): fFileList({file}), fFilePos(fFileList.begin()), fFile(), fReader(), 
                                           fEvent(fReader, "Event")
  {
    //TODO: ReadFile()
    if(fFilePos == fFileList.end())
    {
      throw std::runtime_error("No files in Source.\n");
    }

    std::cout << "Opening file " << *fFilePos << "\n";

    fFile.reset(TFile::Open((fFilePos)->c_str()));
    fReader.SetTree("EDepSimEvents", fFile.get());
    fReader.GetTree()->BuildIndex("RunId", "EventId");

    auto geo = (TGeoManager*)fFile->Get("EDepSimGeometry");
    if(geo == nullptr) throw std::runtime_error("Failed to get geometry object from file named "+std::string(fFile->GetName())+"\n");
    fGeo = geo;
    //fReader.Next();
  }

  const TG4Event& Source::Event()
  {
    return *fEvent;
  }

  TGeoManager* Source::Geo()
  {
    return fGeo;
  }

  bool Source::Next()
  {
    const bool status = fReader.Next();
    if(!status) 
    {
      fReader.Restart(); //Make sure the TTreeReader keeps working even if this is the end of the file.  
      fReader.Next();
    }
    return status;
  }

  //Go to event by entry number in TTree
  bool Source::GoTo(const size_t evt)
  {
    //TODO: GoTo can't search over file boundaries.  Should GoTo have an argument for the file to process?
    //      std::map::lower_bound could help me search for the right file based on starting entry numbers.  
    if(fReader.GetEntryStatus() == TTreeReader::kEntryBeyondEnd) //"Recover" from reading trying to read the last entry of the current file
    {
      fReader.Restart();
      fReader.Next();
    }
    return fReader.SetEntry(evt) == TTreeReader::kEntryValid;
  }

  //Go to event by RunId and EventId
  bool Source::GoTo(const int run, const int evt)
  {
    if(fReader.GetEntryStatus() == TTreeReader::kEntryBeyondEnd)
    {
      fReader.Restart();
      fReader.Next();
    }
    auto index = fReader.GetTree()->GetEntryNumberWithBestIndex(run, evt);
    return fReader.SetEntry(index) == TTreeReader::kEntryValid;
  }

  const size_t Source::Entry()
  {
    return fReader.GetCurrentEntry();
  }

  const int Source::RunID()
  {
    return fEvent->RunId;
  }
  
  const int Source::EventID()
  {
    return fEvent->EventId;
  }

  const std::string Source::GetFile()
  {
    return std::string(fFile->GetName());
  }

  //TODO: Expose an iterator to file name and beginning and end of list here?  This would allow me to GoTo a specific file and 
  //      simplify the Source interface somewhat.  GetFile() wouldn't be necessary for example.
  //TODO: Undefined behavior seems to happen when this function is called.  It may or may not be the cause.
  bool Source::NextFile()
  {
    ++fFilePos;
    if(fFilePos == fFileList.end()) 
    {
      --fFilePos; //Back away from the edge of disaster
      return false;
    }

    fFile.reset(TFile::Open((fFilePos)->c_str()));
    fReader.SetTree("EDepSimEvents", fFile.get());
    fReader.GetTree()->BuildIndex("RunId", "EventId");

    auto geo = (TGeoManager*)fFile->Get("EDepSimGeometry");
    if(geo == nullptr) throw std::runtime_error("Failed to get geometry object from file named "+std::string(fFile->GetName())+"\n");
    fGeo = geo;
    fReader.Next();
    return true;
  }
}
