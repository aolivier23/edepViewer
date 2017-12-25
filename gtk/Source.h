//File: Source.h
//Brief: A Source for this edepsim display provides access to a TG4Event and a TGeoManager.  It knows how to go to the 
//       next event as well as an arbitrary event offset.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//ROOT includes
#include "TTreeReader.h"
#include "TGeoManager.h"
#include "TChain.h"

//edepsim includes
#include "TG4Event.h"

//c++ includes
#include <memory>

#ifndef SRC_SOURCE_H
#define SRC_SOURCE_H

class TG4Event;
class TGeoManager;

namespace src
{
  class Source
  {
    public:
      Source(const std::vector<std::string>& files);
      Source(const std::string& file);
      virtual ~Source() = default;

      virtual const TG4Event& Event();
      virtual TGeoManager* Geo();

      virtual void Next();
      virtual bool GoTo(const size_t evt);

      virtual const std::string GetFile();
      virtual const size_t Entry();

    protected:
      TChain fTree;
      TTreeReader fReader;
      TTreeReaderValue<TG4Event> fEvent;
  };
}

#endif //SRC_SOURCE_H
