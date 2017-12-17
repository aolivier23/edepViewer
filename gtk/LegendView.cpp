//File: LegendView.cpp
//Brief: A LegendView displays a title followed by rows that show color-name pairs. 
//Author: Andrew Olivier aolivier@ur.rochester.edu

#include "gtk/LegendView.h"

//TODO: Remove me
#include <iostream>

namespace mygl
{
  LegendView::LegendView(const std::string& title, Gtk::Window& parent, std::vector<Row>&& rows): Gtk::Dialog("Legend", parent, false), 
                                                                                                  fTitle(title), fRows(rows)
  {
    get_content_area()->set_orientation(Gtk::Orientation::ORIENTATION_VERTICAL);
    get_content_area()->add(fTitle);

    for(auto& row: fRows) 
    {
      get_content_area()->pack_end(row);
    }

    show_all_children();
  }

  LegendView::Row::Row(const std::string& label, Gdk::RGBA color): fColor(), fLabel(label)
  {
    set_orientation(Gtk::Orientation::ORIENTATION_HORIZONTAL);
    fColorSrc = color;
    fColor.override_background_color(fColorSrc);

    pack_start(fColor);
    pack_start(fLabel);
    show_all_children();
  }

  LegendView::Row::Row(const Row& other): Row(other.fLabel.get_text(), other.fColorSrc)
  {
  }
} 
