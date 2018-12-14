//File: NewFile.h
//Brief: A NewFile is a NonSequential State that wants to load the first event in
//       a different file.  
//Author: Andrew Olivier aolivier@ur.rochester.edu

//Local includes
#include "NewFile.h"
#include "app/Source.h"

//app includes
#include "app/Window.h"

fsm::NewFile::NewFile(const std::string& fileName): NonSequential(), fNextFileName(fileName)
{
}

void fsm::NewFile::seekToEvent(evd::Window& window)
{
  window.SetSource(std::make_unique<src::Source>(fNextFileName));
}
