//File: PlaneCam.cpp
//Brief: A camera that moves in a plane.  Line it up with the vertex you want to look 
//       at using the arrow keys, then zoom in with the scroll wheel.  
//Author: Andrew Olivier aolivier@ur.rochester.edu

//For debugging
#include <iostream>

//glm includes
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

//local includes
#include "PlaneCam.h"

namespace mygl
{
  void PlaneCam::ReCalcView()
  {
    //Look around the scene
    fFront.x = std::cos(glm::radians(fPitch))*std::cos(glm::radians(fYaw));
    fFront.y = std::sin(glm::radians(fPitch));
    fFront.z = std::cos(glm::radians(fPitch))*std::sin(glm::radians(fYaw));
    fFront = glm::normalize(fFront);
  }

  void PlaneCam::do_key_press()
  {    
    //Move the camera position
    const auto right = glm::normalize(glm::cross(fFront, fUp));
    const auto top = glm::normalize(glm::cross(right, fFront));
    if(fUpArr == PlaneCam::keyState::down)  fPosition += fPosSens*top; 
    if(fDwnArr == PlaneCam::keyState::down) fPosition -= fPosSens*top;
    if(fLftArr == PlaneCam::keyState::down) fPosition -= glm::normalize(glm::cross(fFront, fUp))*fPosSens;
    if(fRgtArr == PlaneCam::keyState::down) fPosition += glm::normalize(glm::cross(fFront, fUp))*fPosSens;
  }

  void PlaneCam::do_motion(const std::pair<double, double>& pos)
  {
    fYaw += pos.first - fPrevMousePos.first;
    fPitch += pos.second - fPrevMousePos.second;

    //Make sure pitch cannot get too large
    if(fPitch < -89.) fPitch = -89.;
    if(fPitch > 89.) fPitch = 89.;
  }
}

