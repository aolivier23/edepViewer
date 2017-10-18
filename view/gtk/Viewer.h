//File: Viewer.h
//Brief: A Viewer is a Gtk::Box that contains some visualization information.  
//       Viewers can handle updating themselves when they don't need to interact 
//       with other Viewers, but a controller class should handle updating all 
//       Viewers for "global" events.  A Viewer has a TreeView 
//       of the Drawables it displays.  Elements are added and/or removed 
//       from the list of Drawables by the "global" selection controller.
//TODO: Reorganize so that there is a concept of a global VisID shared between Viewers.
//TODO: More general Model class.  For now, a TFile from EDepSim is my model.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//gtkmm include
#include <gtkmm.h>

#ifndef VIEW_VIEWER_H
#define VIEW_VIEWER_H

class TTree;
class TTreeReaderValue;
class TTreeReaderArray;

namespace view
{
  class Viewer: public Gtk::Box
  {
    public:
      Viewer(TFile& file); 
      virtual ~Viewer() = default;

      //Called by "global" controllers
      virtual void ReadEvent() = 0;
      virtual void ReadGeo() = 0;

      //TODO: Functions to show, hide, and highlight a given VisID
    protected:
      Gtk::TreeView fTreeView; //View of the objects in this Viewer
      Gtk::ScrolledWindow fScroll;
      Glib::RefPtr<Gtk::TreeStore> fRefTreeModel;
      Glib::RefPtr<Gtk::TreeSelection> fSelection;
  };
}

#endif //VIEW_VIEWER_H
