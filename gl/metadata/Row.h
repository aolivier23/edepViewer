//File: Row.h
//Brief: A Row stores string-convertible data from a ColumnModel 
//       that will be associated with a single Drawable.    
//Author: Andrew Olivier aolivier@ur.rochester.edu

//c++ includes
#include <list>
#include <vector>
#include <memory>
#include <sstream>

#ifndef CTRL_ROW_H
#define CTRL_ROW_H

namespace ctrl
{
  class ColumnModel;

  template <class T>
  class Column;

  namespace detail
  {
    class DataBase;
  }

  //A ctrl::Row holds data associated with a particular Drawable.  
  //To be stored in a ctrl::Row, a data type just has to be convertible 
  //to a std::string.  
  class Row
  {
    public:
      Row(const ColumnModel& cols);
      virtual ~Row();

      //Explicitly delete copy facilities
      Row(const Row& other) = delete;
      Row& operator=(Row other) = delete;
                                                                                                                        
      //Access to Row data using Columns
      template <class T>
      T& operator[](const Column<T>& col)
      {
        return *((T*)(fColData[col.GetPosition()]->Get()));
      }
                                                                                                                        
      //Access to Row data as strings
      std::string operator [](const size_t index) const;
                                                                                                                        
    private:
      std::vector<std::unique_ptr<detail::DataBase>> fColData; //Vector of data indexed by column number
  };
}

#endif //CTRL_ROW_H
