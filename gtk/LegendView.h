//File: LegendView.cpp
//Brief: A LegendView displays a mapping from an image to a name.  A LegendView should 
//       be constructed only when it is needed as the user cannot update it once its 
//       constructor has run.  
//Author: Andrew Olivier aolivier@ur.rochester.edu

//gtkmm includes
#include <gtkmm.h>

#ifndef MYGL_LEGENDVIEW_H
#define MYGL_LEGENDVIEW_H

namespace mygl
{
  class LegendView: public Gtk::Dialog
  {
    public:
      //A Row is one entry in a legend. It displays a color and has a label. 
      class Row: public Gtk::Box
      {
        public:
          Row(const std::string& label, Gdk::RGBA color);
          Row(const Row& other);

        protected:
          Gtk::Label fLabel;
          Gtk::Box   fColor;
          Gdk::RGBA  fColorSrc;
      };

      LegendView(Gtk::Window& parent, std::vector<Row>&& rows);

      virtual ~LegendView() = default;

    protected: 
      std::vector<Row> fRows;
  };
}

#endif //MYGL_LEGENDVIEW_H
