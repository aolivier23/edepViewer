//File: Camera.h
//Brief: The Camera interface reads the user's input so that derived classes can work with it more easily.  
//       The Camera interface is the controller component for interactions with a Gtk::GLArea.  
//       Classes that derive from Camera must provide the part of the Model that handles the logic for 
//       how an opengl scene is viewed.  
//       Camera now provides an additional way for the user to interact with it: it is a GUI component.  
//       If additional properties are added to a Camera, those properties should be given GUI manifestations.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//local includes
#include "gl/camera/Vec3Entry.h"

//c++ includes
#include <utility>

//glm includes
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

//gtkmm includes
#include <gtkmm.h>

#ifndef MYGL_CAMERA_H
#define MYGL_CAMERA_H

namespace mygl
{
  class Camera: public Gtk::Box
  {
    public:
      //Specifying fFront here might not do anything if a derived class recalculates fFront in ReCalcView().
      Camera(const glm::vec3& pos, const glm::vec3& up = glm::vec3(0.0f, 1.0f, 0.0f)): Gtk::Box(Gtk::ORIENTATION_VERTICAL), 
             fModified(true), fPosition(pos), fUp(up), fFront(0.0f, 0.0f, -1.0f), fPosEntry("Position:", fPosition), 
             fTargetEntry("Target:", fPosition+fFront), fUpArr(keyState::up), fDwnArr(keyState::up), fLftArr(keyState::up), 
             fRgtArr(keyState::up), fMouse(keyState::up) 
      {
        pack_start(fPosEntry);
        fPosEntry.signal_activate().connect(sigc::mem_fun(*this, &Camera::UpdatePosition));
        pack_start(fTargetEntry);
        fTargetEntry.signal_activate().connect(sigc::mem_fun(*this, &Camera::UpdateTarget));
        show_all_children();
      }
      virtual ~Camera() = default; 

      //Results for the GL renderer that every Camera must produce
      glm::mat4 GetView(); 
      virtual glm::mat4 GetPerspective(const int width, const int height) = 0;

      //Record results of signals from user that came from GLArea.
      bool on_key_press(const GdkEventKey* evt);
      bool on_key_release(const GdkEventKey* evt);
      bool on_button_press(const GdkEventButton* evt);
      bool on_button_release(const GdkEventButton* evt);
      bool on_motion(const GdkEventMotion* evt);
      bool on_scroll(const GdkEventScroll* evt);

      //utility function to connect signals to a GLArea
      void ConnectSignals(Gtk::Widget& widget);

    private:
      bool fModified; //Has the Camera's state been modified since the view was last recalculated?
      glm::mat4 fView; //Cache the view matrix each time it is calculated to save glm::lookAt calls

      void UpdatePosition();
      void UpdateTarget();

    protected:
      //Derived classes can react to signals here
      virtual void do_key_press() {}
      virtual void do_motion(const std::pair<double, double>& pos) {}
      virtual void do_scroll(const double dir) {}

      //Handle for calculation of view matrix
      virtual void ReCalcView() = 0;

      //Camera attributes in space being viewed
      glm::vec3 fPosition; //Vector to the position of this camera
      const glm::vec3 fUp; //The initial up direction for this camera
      glm::vec3 fFront; //Vector to the front of the camera

      //GUI widgets for interface to camera attributes
      Vec3Entry fPosEntry;
      Vec3Entry fTargetEntry;

      //Camera control state
      //Representations of the current Camera state
      enum class keyState { up, down };

      //Arrow keys
      keyState fUpArr;
      keyState fDwnArr;
      keyState fLftArr;
      keyState fRgtArr;

      //Mouse left button
      keyState fMouse; //Position of the mouse's left button
      std::pair<double, double> fPrevMousePos; //Last mouse position registered
  };
}

#endif //MYGL_CAMERA_H
