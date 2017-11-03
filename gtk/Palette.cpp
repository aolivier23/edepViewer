//Palette.cxx
//Simple class to return a new color as far from previous colors on the HSV cylinder as possible.  Keeps colors at a constant "darkness".
//Andrew Olivier aolivier@ur.rochester.edu

//c++ includes
#include <iostream>

//glm includes
#include <glm/glm.hpp>

#ifndef MYGL_PALETTE_CPP
#define MYGL_PALETTE_CPP

namespace mygl
{
  class Palette
  {
    public:
      Palette(const double valMin, const double valMax, const double hMin=270., const double hMax=0., 
              const float s=0.9, const float v=0.7): fS(s), fV(v), fValMin(valMin), fValMax(valMax), fHMin(hMin), fHMax(hMax)
      //hue, saturation, and value; offset in degrees
      { 
      }
      ~Palette() {}
      
      glm::vec3 operator()(const double value)
      { 
        //Get the color we will return to the user
        float r, g, b;
        float hue = (value-fValMin)/(fValMax-fValMin)*(fHMax-fHMin)+fHMin;
        if(hue > 360.) hue = std::fmod(hue, 360);
         
        //Clamp values to min-max rnage
        if(value < fValMin) hue = fHMin;
        if(value > fValMax) hue = fHMax;  

        //Perform HSV to RGB conversion myself because ROOT's method doesn't seem to work.  The formula works though, so I am copying this from TColor.cxx.
        int i;
        float f, p, q, t;
        
        if(fS == 0) { r=g=b=fV; }
        else
        {
          hue /= 60;
          i = (int)floor(hue);
          f=hue-i;
          p=fV*(1-fS);
          q=fV*(1-fS*f);
          t=fV*(1-fS*(1-f));
  
          switch(i)
          {
            case 0: r = fV; g = t, b = p; break;
            case 1: r = q; g = fV; b = p; break;
            case 2: r = p; g = fV; b = t; break;
            case 3: r = p; g = q; b = fV; break;
            case 4: r = t; g = p; b = fV; break;
            default: r = fV; g = p; b = q; break;
          }        
        }
  
        //std::cout << "h=" << fH << ", s=" << fS << " v=" << fV << " r=" << r << " g=" << g << " b=" << b << "\n";

        return glm::vec3(r, g, b);
      }
    
    private:
      const float fS;
      const float fV;
 
      const double fValMin; 
      const double fValMax;
      const double fHMin; 
      const double fHMax;
  };
}

#endif //MYGL_PALETTE_CPP

