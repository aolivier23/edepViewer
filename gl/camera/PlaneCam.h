//File: PlaneCam.h
//Brief: Camera that moves in a plane with the arrow keys.  Can be rotated with the mouse and 
//       zoomed with the scroll wheel, but cannot (easily) move towards an object
//Author: Andrew Olivier aolivier@ur.rochester.edu

//local includes
#include "OrthoCamera.h"

//c++ includes
#include <utility>
#include <string>

//glm includes
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#ifndef MYGL_PLANECAM_H
#define MYGL_PLANECAM_H

namespace mygl
{
  class PlaneCam: public OrthoCamera
  {
    public:
      PlaneCam(const glm::vec3& pos, const glm::vec3& front = glm::vec3(0.f, 0.f, -1.f), const glm::vec3& up = glm::vec3(0.0f, 1.0f, 0.0f), const float farPlane = 10000., 
               const float posStep = 50., const float zoom = 1.0): OrthoCamera(pos, front, up, farPlane, zoom), fPitch(0), fYaw(0), fPosSens(posStep)
      { 
      }

      virtual ~PlaneCam() = default;

    protected:
      virtual void ReCalcView() override;
      virtual void do_motion(const std::pair<double, double>& pos) override;
      virtual void do_key_press() override;

      virtual void do_render() override;

    private:
      double fPitch;
      double fYaw;
      float fPosSens; //Sensitivity of translations in position to a single arrow key press

      void set_move_speed();
  };
}

#endif //MYGL_PLANECAM_H
