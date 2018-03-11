//File: TreeModel.cpp
//Brief: A TreeModel stores Nodes that each contain a user-defined NTuple of information.  Provides STL-style iterators 
//       that allow traversing the tree's structure.  Meant to replace Gtk::TreeModel.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//include header
#include "TreeModel.h"

namespace mygl
{
  //ColumnModel function definitions
  std::vector<std::unique_ptr<TreeModel::Node::DataBase>> TreeModel::ColumnModel::BuildData()
  {
    std::vector<std::unique_ptr<TreeModel::Node::DataBase>> data;
    for(const auto& col: fCols) data.push_back(col->BuildData());
    return data;
  }

  void TreeModel::ColumnModel::Add(TreeModel::ColumnBase& col)
  {
    col.SetPosition(size());
    fCols.push_back(&col);
  }

  size_t TreeModel::ColumnModel::size() const
  {
    return fCols.size();
  }

  //ColumnBase function definitions
  TreeModel::ColumnBase::ColumnBase(const std::string& name): fName(name)
  {
  }

  //Node function definitions
  TreeModel::Node::Node(const ColumnModel& cols): fChildren(), fColData(cols.BuildData())
  {
  }

}

