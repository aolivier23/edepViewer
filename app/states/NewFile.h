//File: NewFile.h
//Brief: A NewFile is a NonSequential State that wants to load the first event in
//       a different file.  
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef FSM_NEWFILE_H
#define FSM_NEWFILE_H

//Local includes
#include "NonSequential.h"

namespace fsm
{
  class NewFile: public NonSequential
  {
    public:
      NewFile(const std::string& fileName);
      virtual ~NewFile() = default;

    protected:
      virtual void seekToEvent(evd::Window& window) override;

      const std::string fNextFileName; //Name of the next file to process
  };
}

#endif //FSM_NEWFILE_H
