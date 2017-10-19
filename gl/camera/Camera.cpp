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
      std::cout << "In mygl::Camera::GetView(), recalculated the view matrix before drawing.\n";
      ReCalcView();
      //Find the new view matrix
      fView = glm::lookAt(fPosition, fPosition+fFront, fUp);
      fModified = false;
    }
    return fView;
  }

  bool Camera::on_key_press(const GdkEventKey* evt)
  {
    std::cout << "mygl::Camera detected key press event.\n";
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
    std::cout << "mygl::Camera detected key release event.\n";
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
    std::cout << "mygl::Camera detected button press event.\n";
    if(evt->button != 1) return false; //1 is normally the left mouse button
    fMouse = Camera::keyState::down;
    fPrevMousePos = std::make_pair(evt->x, evt->y);
    fModified = true;
    return true;
  }

  bool Camera::on_button_release(const GdkEventButton* evt)
  {
    std::cout << "mygl::Camera detected button release event.\n";
    if(evt->button != 1) return false;
    fMouse = Camera::keyState::up;
    fModified = true;
    return true;
  }

  bool Camera::on_motion(const GdkEventMotion* evt)
  {
    std::cout << "mygl::Camera detected motion event.\n";
    if(fMouse != Camera::keyState::down) return false;
    auto pos = std::make_pair(evt->x, evt->y);
    do_motion(pos);
    fPrevMousePos = pos;
    fModified = true;
    return false;
  }

  bool Camera::on_scroll(const GdkEventScroll* evt)
  {
    std::cout << "mygl::Camera detected scroll event.\n";
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
}
