//File: GLViewer.h
//Brief: A GLViewer is a basic GUI environment for drawing Drawables from GLScenes using opengl.  
//       One event display window can have multiple GLViewers (which I intend to eventually derive
//       from an abstract Viewer class).  Each GLViewer has it's own list tree of objects it is drawing 
//       whose rows are constructed from a core TreeStore's rows.  Selecting an object in one GLViewer
//       should cause that object to be selected in all GLViewers (and eventually all Viewer objects). 
//Author: Andrew Olivier aolivier@ur.rochester.edu

//gtkmm include
#include <gtkmm.h>

namespace view
{
  class GLScene;
  class Drawable;

  class GLViewer: public Gtk::Box
  {
    public:
    protected:
      //Subwidgets

      //List of scenes to draw
      std::vector<std::shared_ptr<
  };
}
