//File: OrthoCamera.cpp
//Brief: An FPS-style camera for opengl visualization.  Allows the user to move around the scene with the arrow keys, "drag" the camera with 
//       the mouse, and zoom with the mouse scroll wheel.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//For debugging
#include <iostream>

//local includes
#include "OrthoCamera.h"

namespace mygl
{
  glm::mat4 OrthoCamera::GetPerspective(const int width, const int height)
  {
    std::cout << "fFarPlane is " << fFarPlane << "\n";
    return glm::ortho(-width/2/fZoom, width/2/fZoom, -height/2/fZoom, height/2/fZoom, -fFarPlane, fFarPlane); 
    //TODO: Drawing things behind the camera indicates that something is VERY wrong with my view matrix.  
    //      I think BOTH z and x are "reversed" in some way in the view matrix.  I also noticed that I am adding 
    //      fFront, a normalized vector, to fPosition, an unormalized vector, to get the "target" position for 
    //      glm::lookAt(). 
  }

  void OrthoCamera::do_scroll(const double scrollSign)
  {
    fZoom += fScrollSpeed*scrollSign;

    //Clamp zoom values
    if(fZoom < 1.0*fScrollSpeed) fZoom = 1.0*fScrollSpeed;
    if(fZoom > 45.*fScrollSpeed) fZoom = 45.*fScrollSpeed;
  }
}
