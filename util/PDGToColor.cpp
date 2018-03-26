//File: PDGToColor.cpp
//Brief: A mapping from PDG code to an RGB color.  Comes up with a new color via ColorIter every time 
//       a new PDG code is mapped. 
//Author: Andrew Olivier aolivier@ur.rochester.edu

//util includes
#include "util/PDGToColor.h"

#ifndef MYGL_PDGTOCOLOR_CPP
#define MYGL_PDGTOCOLOR_CPP

namespace mygl
{
  PDGToColor::PDGToColor(): fNextColor(), fPDGToColor()
  {
  }

  glm::vec3 PDGToColor::operator [](const int pdg)
  {
    const auto found = fPDGToColor.find(pdg);
    if(found == fPDGToColor.end()) 
    {
      fPDGToColor[pdg] = fNextColor++;
    }

    return fPDGToColor[pdg];
  }

  std::map<int, glm::vec3>::iterator PDGToColor::begin()
  {
    return fPDGToColor.begin();
  }

  std::map<int, glm::vec3>::iterator PDGToColor::end()
  {
    return fPDGToColor.end();
  }
}

#endif //MYGL_PDGTOCOLOR_CPP
