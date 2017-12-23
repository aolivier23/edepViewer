//File: RunEvdApp.cpp
//Brief: Runs an EvdWindow to visualize edepsim output files.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//evd includes
#include "gtk/EvdApp.cpp"

int main(int argc, char** argv)
{
  auto app = mygl::EvdApp::create(); 

  return app->run(argc, argv);
}
