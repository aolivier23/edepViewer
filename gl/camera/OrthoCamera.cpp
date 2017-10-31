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
    return glm::ortho(-width/2/fZoom, width/2/fZoom, -height/2/fZoom, height/2/fZoom, -fFarPlane, fFarPlane); 
    //return glm::ortho(fLeft/fZoom, fRight/fZoom, fBottom/fZoom, fTop/fZoom, -fFarPlane, fFarPlane);
    //TODO: I am doing something strange by allowing objects to be drawn at -fFarPlane which should be behind the camera.  
    //      After examining the viewer's behavior with and without drawing objects "behind" the camera, I decided that I 
    //      DO want to draw objects behind the camera to prevent cutting off parts of tracks.  
  }

  void OrthoCamera::do_scroll(const double scrollSign)
  {
    fZoom += fScrollSpeed*scrollSign;
    
    //Make sure zoom value never becomes negative
    if(fZoom < 1.0*fScrollSpeed) 
    {
      fZoom = 1.0*fScrollSpeed;

      //Change the scroll speed so the user can continue to zoom in forever
      fScrollSpeed *= 0.1;
    }
    if(fZoom > 30.*fScrollSpeed) 
    {
      fZoom = 30.*fScrollSpeed;
   
      //Increase the scroll speed so the user can zoom farther out.
      fScrollSpeed *= 10.;
    }

    fZoomEntry.set_text(std::to_string(fZoom));
    fSpeedEntry.set_text(std::to_string(fScrollSpeed));
  }

  void OrthoCamera::set_zoom()
  {
    fZoom = std::stof(fZoomEntry.get_text());
  }

  void OrthoCamera::set_speed()
  {
    fScrollSpeed = std::stof(fSpeedEntry.get_text());
  }
}
