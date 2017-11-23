//File: Camera.cpp
//Brief: Implementation of behavior common to camera interfaces for an OpenGL program.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//For debugging
#include <iostream>

//local includes
#include "Camera.h"

namespace mygl
{
  glm::mat4 Camera::GetView()
  {
    if(fModified)
    {
      ReCalcView();
      //Find the new view matrix
      const auto right = glm::normalize(glm::cross(fFront, fUp));
      const auto up = glm::normalize(glm::cross(right, fFront));
      fView = glm::lookAt(fPosition, fPosition+fFront, up);
      /*std::cout << "In mygl::Camrea::GetView(), the view matrix is now:\n";
      for(size_t y = 0; y < 4; ++y)
      {
        std::cout << "(";
        for(size_t x = 0; x < 4; ++x) std::cout << fView[y][x] << ", ";
        std::cout << ")\n";
      }
      std::cout << "fFront is (" << fFront.x << ", " << fFront.y << ", " << fFront.z << ")\n";
      std::cout << "fPosition is (" << fPosition.x << ", " << fPosition.y << ", " << fPosition.z << ")\n";
      std::cout << "right is (" << right.x << ", " << right.y << ", " << right.z << ")\n";
      std::cout << "The initial fUp is (" << fUp.x << ", " << fUp.y << ", " << fUp.z << ")\n";
      std::cout << "The current up vector is (" << up.x << ", " << up.y << ", " << up.z << ")\n";*/
      fModified = false;
      fTargetEntry.set_value(fPosition+fFront);
      fPosEntry.set_value(fPosition);
    }
    return fView;
  }

  bool Camera::on_key_press(const GdkEventKey* evt)
  {
    auto key = evt->keyval;
    if(key == GDK_KEY_Up) fUpArr = Camera::keyState::down;
    else if(key == GDK_KEY_Down) fDwnArr = Camera::keyState::down;
    else if(key == GDK_KEY_Left) fLftArr = Camera::keyState::down;
    else if(key == GDK_KEY_Right) fRgtArr = Camera::keyState::down;
    else return false;
    do_key_press();
    fModified = true;
    return true;
  }
  
  bool Camera::on_key_release(const GdkEventKey* evt)
  {
    auto key = evt->keyval;
    if(key == GDK_KEY_Up) fUpArr = Camera::keyState::up;
    else if(key == GDK_KEY_Down) fDwnArr = Camera::keyState::up;
    else if(key == GDK_KEY_Left) fLftArr = Camera::keyState::up;
    else if(key == GDK_KEY_Right) fRgtArr = Camera::keyState::up;
    else return false;
    fModified = true;
    return true;
  }

  bool Camera::on_button_press(const GdkEventButton* evt)
  {
    if(evt->button != 1) return false; //1 is normally the left mouse button
    fMouse = Camera::keyState::down;
    fPrevMousePos = std::make_pair(evt->x, evt->y);
    fModified = true;
    return false;
  }

  bool Camera::on_button_release(const GdkEventButton* evt)
  {
    if(evt->button != 1) return false;
    fMouse = Camera::keyState::up;
    fModified = true;
    return false;
  }

  bool Camera::on_motion(const GdkEventMotion* evt)
  {
    if(fMouse != Camera::keyState::down) return false;
    auto pos = std::make_pair(evt->x, evt->y);
    do_motion(pos);
    fPrevMousePos = pos;
    fModified = true;
    return false;
  }

  bool Camera::on_scroll(const GdkEventScroll* evt)
  {
    const auto dir = evt->direction;
    const auto scrollSign = (dir==GDK_SCROLL_UP)?1.0:-1.0; //TODO: Are there other options for scroll sign?
    do_scroll(scrollSign);
    fModified = true;
    return false;
  }

  void Camera::ConnectSignals(Gtk::GLArea& area)
  {
    area.signal_key_press_event().connect(sigc::mem_fun(*this, &mygl::Camera::on_key_press), false);
    area.signal_key_release_event().connect(sigc::mem_fun(*this, &mygl::Camera::on_key_release), false);
    area.signal_button_press_event().connect(sigc::mem_fun(*this, &mygl::Camera::on_button_press), false);
    area.signal_button_release_event().connect(sigc::mem_fun(*this, &mygl::Camera::on_button_release), false);
    area.signal_motion_notify_event().connect(sigc::mem_fun(*this, &mygl::Camera::on_motion), false);
    area.signal_scroll_event().connect(sigc::mem_fun(*this, &mygl::Camera::on_scroll), false);
  }

  void Camera::UpdatePosition()
  {
    fPosition = fPosEntry.get_value();
    fModified = true;
  }

  void Camera::UpdateTarget()
  {
    //Give the illusion of setting the camera target while actually working with a front vector that is more convenient for 
    //calculations.
    fFront = fTargetEntry.get_value()-fPosition; //target = fFront+fPosition from GetView() above
    fModified = true;
  }
}
