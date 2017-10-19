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

    return glm::ortho(-width/2/fZoom, width/2/fZoom, -height/2/fZoom, height/2/fZoom, 0.1f, fFarPlane);    
  }

  void OrthoCamera::do_scroll(const double scrollSign)
  {
    fZoom += fScrollSpeed*scrollSign;

    //Clamp zoom values
    if(fZoom < 1.0*fScrollSpeed) fZoom = 1.0*fScrollSpeed;
    if(fZoom > 45.*fScrollSpeed) fZoom = 45.*fScrollSpeed;
  }
}
