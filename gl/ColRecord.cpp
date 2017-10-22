//File: ColRecord.cpp
//Brief: Base class for all Gtk::TreeModel:ColumnRecord-derived classes used with mygl::Scene.  Your ColumnRecord 
//       class should derive from this one.  Provides standard columns that all Scenes know about.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//gl includes
#include "gl/VisID.h"

//gtkmm includes
#include <gtkmm.h>

#ifndef MYGL_COLRECORD_H
#define MYGL_COLRECORD_H

namespace mygl
{
  class ColRecord: public Gtk::TreeModel::ColumnRecord
  {
    public:
      ColRecord()
      {
        add(fDrawSelf);
        add(fVisID);
      }

      //Visible columns
      Gtk::TreeModelColumn<bool> fDrawSelf;

      //Hidden columns
      Gtk::TreeModelColumn<mygl::VisID> fVisID;
  };
}

#endif //MYGL_COLRECORD_H 
