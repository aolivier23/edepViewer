//File: ColRecord.cpp
//Brief: Base class for all Gtk::TreeModel:ColumnRecord-derived classes used with mygl::Scene.  Your ColumnRecord 
//       class should derive from this one.  Provides standard columns that all Scenes know about.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//gl includes
#include "gl/VisID.h"
#include "gl/TreeModel.h"

#ifndef MYGL_COLRECORD_H
#define MYGL_COLRECORD_H

namespace mygl
{
  class ColRecord: public TreeModel::ColumnModel
  {
    public:
      ColRecord(): fDrawSelf("Draw Self"), fVisID("Vis ID")
      {
        Add(fVisID);
        Add(fDrawSelf);
      }

      //Visible columns
      TreeModel::Column<bool> fDrawSelf;

      //Hidden columns
      TreeModel::Column<mygl::VisID> fVisID;
  };
}

#endif //MYGL_COLRECORD_H 
