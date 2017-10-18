//File: PerspCamera.cpp
//Brief: An FPS-style camera for opengl visualization.  Allows the user to move around the scene with the arrow keys, "drag" the camera with 
//       the mouse, and zoom with the mouse scroll wheel.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//For debugging
#include <iostream>

//local includes
#include "PerspCamera.h"

namespace mygl
{
  const glm::mat4 PerspCamera::GetPerspective(const int width, const int height)
  {

    return glm::perspective(fZoom, (float)width/(float)height, 0.1f, fFarPlane);    
  }

  void PerspCamera::do_scroll(const double scrollSign)
  {
    fZoom += fScrollSpeed*scrollSign;

    //Clamp zoom values
    if(fZoom < 1.0*fScrollSpeed) fZoom = 1.0*fScrollSpeed;
    if(fZoom > 45.*fScrollSpeed) fZoom = 45.*fScrollSpeed;
  }
}
