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

//gtkmm includes
#include <gtkmm.h>

namespace mygl
{
  class OrthoCamera: public Camera
  {
    public:
      OrthoCamera(const glm::vec3& pos, const glm::vec3& up = glm::vec3(0.0f, 1.0f, 0.0f), const float farPlane = 1000.): Camera(pos, up), 
             fZoom(1.0), fFarPlane(farPlane), fScrollSpeed(0.1)
      {
        /*fSpeedEntry.set_text(std::to_string(fScrollSpeed));
        fZoomEntry.set_text(std::to_string(fZoom));

        fZoomEntry.signal_activate().connect(sigc::mem_fun(*this, &OrthoCamera::set_zoom));
        fSpeedEntry.signal_activate().connect(sigc::mem_fun(*this, &OrthoCamera::set_speed));

        pack_start(fZoomLabel, Gtk::PACK_SHRINK);
        pack_start(fZoomEntry, Gtk::PACK_SHRINK);
        pack_start(fSpeedLabel, Gtk::PACK_SHRINK);
        pack_start(fSpeedEntry, Gtk::PACK_SHRINK);
        show_all_children(); */
        //TODO: Restore GUI with ImGUI
      }
      virtual ~OrthoCamera() = default;

      virtual glm::mat4 GetPerspective(const int height, const int width);

    private:
      //Data needed for calculating perspective matrix
      float fZoom; //Zoom of the camera
      float fFarPlane; //Farthest away that objects can be rendered
      float fScrollSpeed; //Measure of how fast zooming is

      //Frustrum parameters
      /*double fTop;
      double fBottom;
      double fLeft;
      double fRight;*/

      //GUI for editing fZoom, fFarPlane, top, bottom, left, and right
      /*Gtk::Label fZoomLabel;
      Gtk::Entry fZoomEntry;
      Gtk::Label fSpeedLabel;
      Gtk::Entry fSpeedEntry;*/

      void set_zoom();
      void set_speed();

    protected:
      virtual void ReCalcView() = 0;
      virtual void do_scroll(const double dir) override;

      virtual void do_render() override;
  };
}
