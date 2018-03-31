//File: VisID.cpp
//Brief: Convenience functions for dealing with Drawable identifiers.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//gl includes
#include "gl/VisID.h"
#include "util/GenException.h"

namespace mygl
{
  VisID::VisID(const unsigned char r, const unsigned char g, const unsigned char b): fR(r), fG(g), fB(b)
  {
  }

  VisID::operator glm::vec4() const
  {
    return glm::vec4(fR/255.0f, fG/255.0f, fB/255.0f, 1.0f);
  }

  VisID& VisID::operator ++() //prefix
  {
    //Handle overflows
    const auto max = std::numeric_limits<unsigned char>::max();
    if(fB == max)
    {
      fB = 0;
      if(fG == max)
      {
        fG = 0;
        if(fR == max) throw util::GenException("Max VisID") << "Reached maximum VisID!  Identifiers for Drawables in "
                                                            << "Scenes are no longer unique.  This will interferer with "
                                                            << "object selection.\n";
        else ++fR;
      }
      else ++fG;
    }
    else ++fB;
    return *this;
  }

  VisID VisID::operator ++(int) //postfix
  {
    VisID result = *this;
    ++(*this);
    return result;
  }

  bool VisID::operator <(const VisID& rhs) const
  {
    if(fR != rhs.fR) return (fR < rhs.fR);
    if(fG != rhs.fG) return (fG < rhs.fG);
    return (fB < rhs.fB);
  }

  bool VisID::operator ==(const VisID& rhs) const
  {
    return (fR == rhs.fR) && (fG == rhs.fG) && (fB == rhs.fB);
  }


  //Printing interface for VisID
  std::ostream& operator <<(std::ostream& os, const VisID& id)
  {
    os << "(" << +(id.fR) << ", " << +(id.fG) << ", " << +(id.fB) << ")";
    return os;
  }
}
