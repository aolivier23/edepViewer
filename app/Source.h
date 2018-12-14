//File: Source.h
//Brief: A Source for this edepsim display provides access to a TG4Event and a TGeoManager.  It knows how to go to the 
//       next event as well as an arbitrary event offset.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//ROOT includes
#include "TTreeReader.h"
#include "TGeoManager.h"
#include "TFile.h"
#include "TTree.h"

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

      //metadata for one event provided by this Source
      struct metadata
      {
        metadata(const int event, const int run, const std::string& file, const bool fileChange): eventID(event), runID(run), 
                                                                                                  fileName(file), newFile(fileChange) 
        {
        }
                                                                                                                                      
        int eventID; //Event number from a TG4Event
        int runID; //Run number from a TG4Event
        std::string fileName; //Name of the file used to produce this event
        bool newFile; //Is this the first event in a new file?
      };

      virtual metadata Next();
      virtual metadata GoTo(const int run, const int evt);

      //exception to throw on failing to find the next file
      class no_more_files
      {
        public:
          no_more_files(const std::string& file) noexcept 
          {
            fError = file+" is the last input file in this Source.";
          }

          no_more_files(const no_more_files& other) noexcept: fError(other.fError) {} 
          no_more_files& operator =(const no_more_files& other) noexcept
          {
            fError = other.fError;
            return *this;
          }

          virtual ~no_more_files() = default;

          const char* what() const noexcept
          {
            return fError.c_str();
          }

          std::string fError; //Last file in the Source that threw this exception
      };

    protected:
      virtual metadata Meta(const bool fileChange);
      virtual bool NextFile();

      //Resources for figuring out what file to process next
      std::vector<std::string> fFileList;
      std::vector<std::string>::iterator fNextFile;

      //Resources for the current file
      std::unique_ptr<TFile> fFile;
      TGeoManager* fGeo; //fGeo is managed by fFile, so this can only be an observer pointer

    public:
      //Don't make me regret making this public
      TTreeReader fReader;

    protected:
      TTreeReaderValue<TG4Event> fEvent;
  };
}

#endif //SRC_SOURCE_H
