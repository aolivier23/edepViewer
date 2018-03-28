//File: PDGToColor.h
//Brief: A mapping from PDG code to an RGB color.  Comes up with a new color via ColorIter every time 
//       a new PDG code is mapped. 
//Author: Andrew Olivier aolivier@ur.rochester.edu

//util includes
#include "util/ColorIter.cxx"

//c++ includes
#include <map>

#ifndef MYGL_PDGTOCOLOR_H
#define MYGL_PDGTOCOLOR_H 

namespace mygl
{
  class PDGToColor
  {
    public:
      PDGToColor();
      virtual ~PDGToColor() = default;

      virtual glm::vec3 operator [](const int pdg);

      std::map<int, glm::vec3>::iterator begin();
      std::map<int, glm::vec3>::iterator end();

    protected:
      ColorIter fNextColor;
      std::map<int, glm::vec3> fPDGToColor;
  };
}

#endif //MYGL_PDGTOCOLOR_H
