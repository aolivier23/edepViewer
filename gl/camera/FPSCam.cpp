//File: FPSCam.cpp
//Brief: An FPS-style camera for opengl visualization.  Allows the user to move around the scene with the arrow keys, "drag" the camera with 
//       the mouse, and zoom with the mouse scroll wheel.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//For debugging
#include <iostream>

//glm includes
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

//local includes
#include "FPSCam.h"

namespace mygl
{
  void FPSCam::ReCalcView()
  {
    //Look around the scene
    fFront.x = std::cos(glm::radians(fPitch))*std::cos(glm::radians(fYaw));
    fFront.y = std::sin(glm::radians(fPitch));
    fFront.z = std::cos(glm::radians(fPitch))*std::sin(glm::radians(fYaw));
    fFront = glm::normalize(fFront);
  }

  void FPSCam::do_key_press()
  {    
    //Move the camera position
    if(fUpArr == FPSCam::keyState::down)  fPosition += fPosSens*fFront; //glm problem here
    if(fDwnArr == FPSCam::keyState::down) fPosition -= fPosSens*fFront;
    if(fLftArr == FPSCam::keyState::down) fPosition -= glm::normalize(glm::cross(fFront, fUp))*fPosSens;
    if(fRgtArr == FPSCam::keyState::down) fPosition += glm::normalize(glm::cross(fFront, fUp))*fPosSens;
  }

  void FPSCam::do_motion(const std::pair<double, double>& pos)
  {
    fYaw += pos.first - fPrevMousePos.first;
    fPitch += pos.second - fPrevMousePos.second;

    //Make sure pitch cannot get too large
    if(fPitch < -89.) fPitch = -89.;
    if(fPitch > 89.) fPitch = 89.;
  }
}

