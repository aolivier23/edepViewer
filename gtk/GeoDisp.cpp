//File: GeoDisp.cpp
//Brief: Runs a GeoDispWindow to show how a ROOT geometry can be viewed with GTKMM and OpenGL.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//evd includes
#include "GeoDispWindow.h"

//gtkmm includes
#include "gtkmm/application.h"

int main(int argc, char** argv)
{
  auto app = Gtk::Application::create(argc, argv, "test.GeoDispWindow");
  
  evd::GeoDispWindow window("/home/aolivier/ND_Studies/geo/maindet.gdml");
  
  return app->run(window);
}
