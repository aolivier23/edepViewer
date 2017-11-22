//File: VisID.h
//Brief: An identifier for visualization objects that can be easily converted to a color that opengl can draw.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//glm includes
#include <glm/glm.hpp>

//c++ includes
#include <iostream>

#ifndef MYGL_VISID_H
#define MYGL_VISID_H

namespace mygl
{
  struct VisID
  {
    unsigned char fR;
    unsigned char fG;
    unsigned char fB;
  
    VisID(const unsigned char r=0, const unsigned char g=0, const unsigned char b=0);
  
    operator glm::vec4() const;
  
    VisID& operator ++(); //prefix
    VisID operator ++(int); //postfix
  
    bool operator <(const VisID& rhs) const;

    bool operator ==(const VisID& rhs) const;
  };
  
  std::ostream& operator <<(std::ostream& os, const VisID& id);
}

#endif //MYGL_VISID_H

