//File: GLFWApp.cpp
//Brief: Run the event display with GLFW as the window backend.  Accepts input from the command line, 
//       but it is an error to provide anything other than file names. 
//Author: Andrew Olivier aolivier@ur.rochester.edu

//Acknowledgement: Huge chunks of code taken from Dear IMGUI's opengl3 example to make stuff work with glfw 
//                 quickly.

//GLAD include(s)
#include "glad/include/glad/glad.h"

//imgui includes
//TODO: Figure out how to work with these from an external library or copy them into this application directly.
#include "imgui.h"
#include "examples/opengl3_example/imgui_impl_glfw_gl3.h"

//glfw include(s)
#include <GLFW/glfw3.h> 

//local include
#include "EvdWindow.h"

int main(const int argc, const char** argv)
{
  //glfwSetErrorCallback(error_callback); //TODO: Throw exception here?  
  if (!glfwInit())
      return 1;
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  #if __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  #endif
  auto window = glfwCreateWindow(1280, 720, "edepViewer", nullptr, nullptr);

  glfwMakeContextCurrent(window); //One context to rule them all?
  glfwSwapInterval(1); // Enable vsync

  //Initialize glad to get opengl extensions.  
  gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
  gladLoadGL();

  // Setup ImGui binding
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO(); 
  ImGui_ImplGlfwGL3_Init(window, true);
  io.NavFlags |= ImGuiNavFlags_EnableKeyboard;  // Enable Keyboard Controls
  io.NavFlags |= ImGuiNavFlags_EnableGamepad;   // Enable Gamepad Controls
  io.NavFlags |= ImGuiNavFlags_MoveMouse; //Enable mouse movement

  //TODO: Create event display "Window" object here?  Pass it command line args, or make source and configuration file 
  //      myself?  Leaning towards former option to make switching window creation libraries as easy as possible.  

  // Setup style
  //TODO: All IMGUI stuff from this point onwards needs to become part of some Window class?  Try to reduce boilerplate 
  //      as much as possible in case I need to switch windowing systems one day (i.e. Emscripten?).
  //TODO: Option to choose background style.  Dark for monitors and white for projectors.  
  ImGui::StyleColorsDark();
  //ImGui::StyleColorsClassic();
  
  //TODO: Can I use glm here instead?  
  //TODO: Get this from application Model.
  ImVec4 clear_color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

  mygl::EvdWindow evd; //evd(argc, argv);

  //TODO: Move this back into EvdWindow
  std::unique_ptr<tinyxml2::XMLDocument> config(new tinyxml2::XMLDocument());
  const auto status = config->LoadFile("default.xml");
  if(status != tinyxml2::XML_SUCCESS)
  {
    throw std::runtime_error("Got error "+std::to_string(status)+" when trying to load configuration file default.xml with tinyxml2.\n");
  }
  evd.reconfigure(std::move(config));
  evd.SetSource(std::unique_ptr<src::Source>(new src::Source("/home/aolivier/ND_Studies/neutrons/clusters_new.root")));

  evd.make_scenes();

  //Rendering loop.  Needs to depend on library providing the opengl context/window.
  while (!glfwWindowShouldClose(window))
  {
    //Helpful comments from IMGUI example:
    // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
    // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
    // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
    // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
    glfwPollEvents(); //TODO: Since this is an event viewer rather than a game, use glfwWaitEvents() instead?
    ImGui_ImplGlfwGL3_NewFrame();

    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    //TODO: Does this mean that Viewer needs to be rendered first?
    /*glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT); */

    //TODO: Render my event display stuff here.  Application Model probably has a Render() function 
    //      that is expected to be called in rendering loop.
    evd.Render(display_w, display_h, io);

    ImGui::Render();
    ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window);
  }

  // Cleanup
  ImGui_ImplGlfwGL3_Shutdown();
  ImGui::DestroyContext();
  glfwTerminate();

  //TODO: Catch exceptions and report errors appropriately.

  //Return 0 on success 
  return 0;
}
