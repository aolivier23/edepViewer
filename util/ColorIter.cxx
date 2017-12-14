//ColorIter.cxx
//Simple class to return a new color as far from previous colors on the HSV cylinder as possible.  Keeps colors at a constant "darkness".
//Andrew Olivier aolivier@ur.rochester.edu

//c++ includes
#include <iostream>

//glm includes
#include <glm/glm.hpp>

namespace mygl
{
  class ColorIter
  {
    public:
      ColorIter(float h=0.0, float s=0.9, float v=0.7, float distance = 100.): fH(h), fS(s), fV(v), fDistance(distance), fFirstReset(false)
      //hue, saturation, and value; offset in degrees
      { 
        fOffset = fDistance; //could we use modulus instead of fOffset here?
        (*this)(); //Initialize fColor.   
      }
      ~ColorIter() {}
      
      ColorIter& operator++()
      { 
        //Get the color we will return to the user
        float r, g, b, hue = fH, satur = fS, value = fV;
  
        //Perform HSV to RGB conversion myself because ROOT's method doesn't seem to work.  The formula works though, so I am copying this from TColor.cxx.
        int i;
        float f, p, q, t;
        
        if(satur == 0) { r=g=b=value; }
        else
        {
          hue /= 60;
          i = (int)floor(hue);
          f=hue-i;
          p=value*(1-satur);
          q=value*(1-satur*f);
          t=value*(1-satur*(1-f));
  
          switch(i)
          {
            case 0: r = value; g = t, b = p; break;
            case 1: r = q; g = value; b = p; break;
            case 2: r = p; g = value; b = t; break;
            case 3: r = p; g = q; b = value; break;
            case 4: r = t; g = p; b = value; break;
            default: r = value; g = p; b = q; break;
          }        
        }
  
        //std::cout << "h=" << fH << ", s=" << fS << " v=" << fV << " r=" << r << " g=" << g << " b=" << b << "\n";

        fColor = glm::vec3(r, g, b);
        
        //Update for next color request
        if(fH + fDistance < 360.) fH += fDistance;
        else
        {
          fOffset /= 2.;
          if(fFirstReset) fDistance /= 2.;
          fH = fOffset;
          fFirstReset = true;
        }
        return *this; //color;
      }
    
      ColorIter operator++(int)
      {
        ColorIter result(*this);
        ++(*this);
        return result;
      }
     
      glm::vec3 operator()()
      {
        ++(*this);
        return (*this);
      }
  
      operator glm::vec3() //Now, we can use a ColorIter object directly where ROOT expects a color index!  
      {
        return fColor;
      }
  
    private:
      float fH;
      const float fS;
      const float fV;
      float fDistance;
      bool fFirstReset; //Has this color iterator been reset the first time?  If so, start dividing fDistance by 2.
      float fOffset;
      glm::vec3 fColor; //The color represented by this iterator.  Stored as (r, g, b)
  };
}

