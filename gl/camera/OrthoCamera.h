//File: OrthoCamera.h
//Brief: A camera with zoom capability.  Implements GetOrthoective and deals with zoom.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//local includes
#include "gl/camera/Camera.h"

//c++ includes
#include <utility>
#include <string>

//glm includes
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace mygl
{
  class OrthoCamera: public Camera
  {
    public:
      OrthoCamera(const glm::vec3& pos, const glm::vec3& front, const glm::vec3& up = glm::vec3(0.0f, 1.0f, 0.0f), const float farPlane = 1000., 
                  const float zoom = 1.0): Camera(pos, front, up), 
             fZoom(zoom), fFarPlane(farPlane), fScrollSpeed(0.1)
      {
      }
      virtual ~OrthoCamera() = default;

      virtual glm::mat4 GetPerspective(const int height, const int width);

    private:
      //Data needed for calculating perspective matrix
      float fZoom; //Zoom of the camera
      float fFarPlane; //Farthest away that objects can be rendered
      float fScrollSpeed; //Measure of how fast zooming is

    protected:
      virtual void ReCalcView() = 0;
      virtual void do_scroll(const double dir) override;

      virtual void do_render() override;
  };
}
