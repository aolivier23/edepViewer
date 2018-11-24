//File: Row.cpp
//Brief: A Row stores string-convertible data from a ColumnModel 
//       that will be associated with a single Drawable.    
//Author: Andrew Olivier aolivier@ur.rochester.edu

//c++ includes
#include <list>
#include <vector>
#include <memory>
#include <sstream>

//ctrl includes
#include "Row.h"
#include "Column.cpp"
#include "Data.cpp"

namespace ctrl
{
  Row::Row(const ColumnModel& cols): fColData(cols.BuildData())
  {
  }

  Row::~Row() {}

  //Access to Row data as strings
  std::string Row::operator [](const size_t index) const
  {
    return fColData[index]->string();
  }
}
