//File: TreeModel.cpp
//Brief: A TreeModel stores Nodes that each contain a user-defined NTuple of information.  Provides STL-style iterators 
//       that allow traversing the tree's structure.  Meant to replace Gtk::TreeModel.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//include header
#include "TreeModel.h"

namespace mygl
{
  //ColumnModel function definitions
  std::vector<std::unique_ptr<TreeModel::Node::DataBase>> TreeModel::ColumnModel::BuildData() const
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

  std::string TreeModel::Node::operator [](const size_t index)
  {
    return fColData[index]->string();
  }

  //iterator function definitions
  TreeModel::iterator::iterator(Base node): fNode(node)
  {
  }

  TreeModel::Node& TreeModel::iterator::operator *() const
  {
    return *(*fNode);
  }
  
  TreeModel::Node* TreeModel::iterator::operator ->() const
  {
    return &*(*fNode);
  }

  TreeModel::iterator TreeModel::iterator::operator ++()
  {
    ++fNode;
    return *this;
  }

  TreeModel::iterator TreeModel::iterator::operator ++(int)
  {
    auto copy = *this;
    ++fNode;
    return copy;
  }

  /*TreeModel::iterator TreeModel::iterator::operator +(const int dist)
  {
    auto copy = *this;
    copy.fNode += dist;
    return copy;
  }*/

  //TODO: Needs parent
  /*operator (bool)() const
  {
    return fNode == 
  }*/

  /*bool TreeModel::iterator::operator <(const iterator& other) const
  { 
    return fNode < other.fNode;
  }*/

  bool TreeModel::iterator::operator ==(const iterator& other) const
  {
    return fNode == other.fNode;
  }

  //const_iterator functions  
  TreeModel::const_iterator::const_iterator(Base node): fNode(node)
  {
  }
                                                               
  const TreeModel::Node& TreeModel::const_iterator::operator *() const
  {
    return *(*fNode);
  }
  
  const TreeModel::Node* TreeModel::const_iterator::operator ->() const
  {
    return &*(*fNode);
  }
                                                               
  TreeModel::const_iterator TreeModel::const_iterator::operator ++()
  {
    ++fNode;
    return *this;
  }
                                                               
  TreeModel::const_iterator TreeModel::const_iterator::operator ++(int)
  {
    auto copy = *this;
    ++fNode;
    return copy;
  }
                                                               
  //TODO: Needs parent
  /*operator (bool)() const
  {
    return fNode == 
  }*/
                                                               
  /*bool TreeModel::const_iterator::operator <(const const_iterator& other) const
  { 
    return fNode < other.fNode;
  }*/
                                                               
  bool TreeModel::const_iterator::operator ==(const const_iterator& other) const
  {
    return fNode == other.fNode;
  }
}

