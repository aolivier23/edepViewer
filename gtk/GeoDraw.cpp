//File: GeoDraw.cpp
//Brief: Runs a GeoDrawWindow to show how a ROOT geometry can be viewed with GTKMM and OpenGL.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//evd includes
#include "GeoDrawWindow.h"

//gtkmm includes
#include "gtkmm/application.h"

int main(int argc, char** argv)
{
  auto app = Gtk::Application::create(argc, argv, "test.GeoDrawWindow");
  
  evd::GeoDrawWindow window("/home/aolivier/ND_Studies/geo/maindet.gdml");
  
  return app->run(window);
}
