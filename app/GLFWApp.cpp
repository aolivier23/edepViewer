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
#include "CmdLine.cpp"

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
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls
  //io.ConfigFlags |= ImGuiConfigFlags_MoveMouse; //Enable mouse movement

  //TODO: Create event display "Window" object here?  Pass it command line args, or make source and configuration file 
  //      myself?  Leaning towards former option to make switching window creation libraries as easy as possible.  

  // Setup style
  //TODO: Make style stuff either part of Window class or write it in a function with style default values.  
  //TODO: All IMGUI stuff from this point onwards needs to become part of some Window class?  Try to reduce boilerplate 
  //      as much as possible in case I need to switch windowing systems one day (i.e. Emscripten?).
  //ImGui::StyleColorsDark();
  ImGui::StyleColorsLight();
  ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(209./255., 209./255., 209./255., 1));
  
  io.Fonts->AddFontFromFileTTF(FONT_DIR "/Roboto-Medium.ttf", 18.0f); //Because the default font is not as readable
  
  //TODO: Can I use glm here instead?  
  //TODO: Get this from application Model.
  ImVec4 clear_color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

  mygl::EvdWindow evd; //evd(argc, argv);

  //Parse the command line to configure evd window
  evd.reconfigure(cmd::FindConfig(argc, argv));
  evd.SetSource(cmd::FindSource(argc, argv));

  //evd.make_scenes();

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

    evd.Render(display_w, display_h, io);

    //#if defined(DEBUG) //TODO: Does this work with non-GCC compilers?
      //bool showMetrics = true;
      //ImGui::ShowMetricsWindow(&showMetrics);
    //#endif

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
