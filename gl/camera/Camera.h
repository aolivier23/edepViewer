//File: Camera.h
//Brief: The Camera interface reads the user's input so that derived classes can work with it more easily.  
//       The Camera interface is the controller component for interactions with a Gtk::GLArea.  
//       Classes that derive from Camera must provide the part of the Model that handles the logic for 
//       how an opengl scene is viewed.    
//Author: Andrew Olivier aolivier@ur.rochester.edu

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
  class Camera
  {
    public:
      Camera(const glm::vec3& pos, const glm::vec3& up = glm::vec3(0.0f, 1.0f, 0.0f)):
             fModified(true), fPosition(pos), fUp(up), fFront(0.0f, 0.0f, 1.0f),
             fUpArr(keyState::up), fDwnArr(keyState::up), fLftArr(keyState::up), fRgtArr(keyState::up), 
             fMouse(keyState::up) {}
      virtual ~Camera() = default; //TODO: Call DisconnectSignals when implemented

      //Results for the GL renderer that every Camera must produce
      const glm::mat4 GetView(); 
      virtual const glm::mat4 GetPerspective(const int width, const int height) = 0;

      //Record results of signals from user that came from GLArea.
      bool on_key_press(const GdkEventKey* evt);
      bool on_key_release(const GdkEventKey* evt);
      bool on_button_press(const GdkEventButton* evt);
      bool on_button_release(const GdkEventButton* evt);
      bool on_motion(const GdkEventMotion* evt);
      bool on_scroll(const GdkEventScroll* evt);

      //utility function to connect signals to a GLArea
      void ConnectSignals(Gtk::GLArea& area);
      //TODO: DisconnectSignals()

    private:
      bool fModified; //Has the Camera's state been modified since the view was last recalculated?
      glm::mat4 fView; //Cache the view matrix each time it is calculated to save glm::lookAt calls

    protected:
      //Derived classes can react to signals here
      virtual void do_key_press() {}
      virtual void do_motion(const std::pair<double, double>& pos) {}
      virtual void do_scroll(const double dir) {}

      //Handle for calculation of view matrix
      virtual void ReCalcView() = 0;

      //Camera attributes in space being viewed
      glm::vec3 fPosition; //Vector to the position of this camera
      const glm::vec3 fUp; //The up direction for this camera
      glm::vec3 fFront; //Vector to the front of the camera

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
