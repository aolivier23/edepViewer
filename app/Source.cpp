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
  }

  const TG4Event& Source::Event()
  {
    return *fEvent;
  }

  TGeoManager* Source::Geo()
  {
    return fGeo;
  }

  Source::metadata Source::Next()
  {
    bool fileChange = false;
    do
    {
      if(fReader.Next()) return Meta(fileChange);
    } 
    while((fileChange = NextFile()));

    //Make sure the TTreeReader keeps working even if this is the end of the file.  
    fReader.Restart();
    fReader.Next();

    //If we get here, we're out of files
    throw no_more_files(fFileList.back());
  }

  //Go to event by RunId and EventId
  Source::metadata Source::GoTo(const int run, const int evt)
  {
    //TODO: Do I still need this check?
    if(fReader.GetEntryStatus() == TTreeReader::kEntryBeyondEnd)
    {
      fReader.Restart();
      fReader.Next();
    }

    bool fileChange = false;
    do
    {
      auto index = fReader.GetTree()->GetEntryNumberWithBestIndex(run, evt);
      if(fReader.SetEntry(index) == TTreeReader::kEntryValid && NextFile()) return Meta(fileChange);
    }
    while((fileChange = NextFile()));

    //If we get here, we're out of files.  
    throw no_more_files(fFileList.back());
  }

  Source::metadata Source::Meta(const bool fileChange)
  {
    return metadata(fEvent->EventId, fEvent->RunId, fFile->GetName(), fileChange);
  }

  //Try to go to the next file in fFileList.  Return false if there are no more files.  
  //You must call fReader.Next() between using this function and dereferencing fEvent.
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
    //fReader.Next(); //TODO: Do I need this anymore?
    return true;
  }
}
