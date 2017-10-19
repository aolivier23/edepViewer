//File: PerspCamera.h
//Brief: A camera with zoom capability.  Implements GetPerspective and deals with zoom.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//local includes
#include "Camera.h"

//c++ includes
#include <utility>
//#include <chrono> //std::chrono::system_clock appears to be very slow
#include <ctime>

//glm includes
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

//gtkmm includes
#include <gtkmm.h>

namespace mygl
{
  class PerspCamera: public Camera
  {
    public:
      PerspCamera(const glm::vec3& pos, const glm::vec3& up = glm::vec3(0.0f, 1.0f, 0.0f), const float farPlane = 1000.): Camera(pos, up), 
             fZoom(1.0), fFarPlane(farPlane), fScrollSpeed(0.1) { }
      virtual ~PerspCamera() = default;

      virtual glm::mat4 GetPerspective(const int height, const int width);

    private:
      //Data needed for calculating perspective matrix
      float fZoom; //Zoom of the camera
      float fFarPlane; //Farthest away that objects can be rendered
      double fScrollSpeed; //Measure of how fast zooming is

    protected:
      virtual void ReCalcView() = 0;
      virtual void do_scroll(const double dir) override;
  };
}
