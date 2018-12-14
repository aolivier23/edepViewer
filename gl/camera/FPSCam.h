//File: FPSCam.h
//Brief: First-person shooter-style camera that can be moved with the arrow keys, aimed with the mouse, and 
//       zoomed with the scroll wheel.  Implements the Camera interface.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//local includes
#include "OrthoCamera.h"

//c++ includes
#include <utility>
//#include <chrono> //std::chrono::system_clock appears to be very slow
#include <ctime>

//glm includes
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#ifndef MYGL_FPSCAM_H
#define MYGL_FPSCAM_H

namespace mygl
{
  class FPSCam: public OrthoCamera
  {
    public:
      FPSCam(const glm::vec3& pos, const glm::vec3& front, const glm::vec3& up = glm::vec3(0.0f, 1.0f, 0.0f), const float farPlane = 10000., 
             float posStep = 50.): OrthoCamera(pos, front, up, farPlane), fPosSens(posStep) { }
      virtual ~FPSCam() = default;

    protected:
      virtual void ReCalcView() override;
      virtual void do_motion(const std::pair<double, double>& pos) override;
      virtual void do_key_press() override;

    private:
      double fPitch;
      double fYaw;
      const float fPosSens; //Sensitivity of translations in position to a single arrow key press
  };
}

#endif //MYGL_FPSCAM_H
