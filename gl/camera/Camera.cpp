//File: Camera.cpp
//Brief: Implementation of behavior common to camera interfaces for an OpenGL program.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//For debugging
#include <iostream>

//imgui includes
#include "imgui.h"

//glfw includes
#include <GLFW/glfw3.h>

//glm includes
#include <glm/gtc/type_ptr.hpp>

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
      //TODO: ImGUI
      //fTargetEntry.set_value(fPosition+fFront);
      //fPosEntry.set_value(fPosition);
    }
    return fView;
  }

  bool Camera::on_key_press(const int key)
  {
    if(key == GLFW_KEY_UP) fUpArr = Camera::keyState::down;
    else if(key == GLFW_KEY_DOWN) fDwnArr = Camera::keyState::down;
    else if(key == GLFW_KEY_LEFT) fLftArr = Camera::keyState::down;
    else if(key == GLFW_KEY_RIGHT) fRgtArr = Camera::keyState::down;
    else return false;
    do_key_press();
    fModified = true;
    return true;
  }
  
  //TODO: Replace GDK with GLFW
  bool Camera::on_key_release(const int key)
  {
    if(key == GLFW_KEY_UP) fUpArr = Camera::keyState::up;
    else if(key == GLFW_KEY_DOWN) fDwnArr = Camera::keyState::up;
    else if(key == GLFW_KEY_LEFT) fLftArr = Camera::keyState::up;
    else if(key == GLFW_KEY_RIGHT) fRgtArr = Camera::keyState::up;
    else return false;
    fModified = true;
    return true;
  }

  bool Camera::on_button_press(const int button, const float x, const float y)
  {
    if(button != 0) return false; //1 is normally the left mouse button
    fMouse = Camera::keyState::down;
    fPrevMousePos = std::make_pair(x, y);
    fModified = true;
    return false;
  }

  bool Camera::on_button_release(const int button)
  {
    if(button != 0) return false;
    fMouse = Camera::keyState::up;
    fModified = true;
    return false;
  }

  bool Camera::on_motion(const float x, const float y)
  {
    if(fMouse != Camera::keyState::down) return false;
    auto pos = std::make_pair(x, y);
    do_motion(pos);
    fPrevMousePos = pos;
    fModified = true;
    return false;
  }

  bool Camera::on_scroll(const float dist)
  {
    const auto scrollSign = (dist > 0)?1.0:-1.0; //TODO: Are there other options for scroll sign?
    do_scroll(scrollSign);
    fModified = true;
    return false;
  }

  /*void Camera::ConnectSignals(Gtk::GLArea& area)
  {
    area.signal_key_press_event().connect(sigc::mem_fun(*this, &mygl::Camera::on_key_press), false);
    area.signal_key_release_event().connect(sigc::mem_fun(*this, &mygl::Camera::on_key_release), false);
    area.signal_button_press_event().connect(sigc::mem_fun(*this, &mygl::Camera::on_button_press), false);
    area.signal_button_release_event().connect(sigc::mem_fun(*this, &mygl::Camera::on_button_release), false);
    area.signal_motion_notify_event().connect(sigc::mem_fun(*this, &mygl::Camera::on_motion), false);
    area.signal_scroll_event().connect(sigc::mem_fun(*this, &mygl::Camera::on_scroll), false);
  }*/

  void Camera::update(const ImGuiIO& io)
  {
    //Adjust camera state based on user input
    if(!io.WantCaptureMouse) //Make sure imgui is not handling mouse
    {
      //TODO: If I want other buttons, handle them here
      if(ImGui::IsMouseClicked(0)) on_button_press(0, io.MousePos.x, io.MousePos.y);
      else if(ImGui::IsMouseReleased(0)) on_button_release(0);
      if(ImGui::IsMouseDragging()) on_motion(io.MousePos.x, io.MousePos.y);
      if(io.MouseWheel != 0.) on_scroll(io.MouseWheel);
    }
    //TODO: Remove dependence on GLFW for interpretting keys?  Probably means I'll need to make some changes to 
    //      the imgui initialization function I got for free.
    if(!io.WantCaptureKeyboard) //Make sure imgui is not handling keyboard
    {
      if(ImGui::IsKeyPressed(GLFW_KEY_RIGHT)) on_key_press(GLFW_KEY_RIGHT);
      else if(fRgtArr == keyState::down) on_key_release(GLFW_KEY_RIGHT);
      if(ImGui::IsKeyPressed(GLFW_KEY_LEFT)) on_key_press(GLFW_KEY_LEFT);
      else if(fLftArr == keyState::down) on_key_release(GLFW_KEY_LEFT);
      if(ImGui::IsKeyPressed(GLFW_KEY_UP)) on_key_press(GLFW_KEY_UP);
      else if(fUpArr == keyState::down) on_key_release(GLFW_KEY_UP);
      if(ImGui::IsKeyPressed(GLFW_KEY_DOWN)) on_key_press(GLFW_KEY_DOWN);
      else if(fDwnArr == keyState::down) on_key_release(GLFW_KEY_DOWN);
    }
  }

  //TODO: Keeping this for backwards compatibility for now
  void Camera::render()
  {
    do_render();
  }

  void Camera::do_render()
  {
    //Call imgui functions so the user can reconfigure the camera.  The current camera draws imgui widgets in whatever window 
    //is currently available.  Currently, the Viewer provides that window
    //Render and configure the camera's current target
    auto target = fFront+fPosition;
    if(ImGui::InputFloat3("Target [mm]", glm::value_ptr(target))) fFront = target-fPosition;

    //Render and configure position
    ImGui::InputFloat3("Position [mm]", glm::value_ptr(fPosition));
  }
}
