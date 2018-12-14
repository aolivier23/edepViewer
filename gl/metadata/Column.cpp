//File: Column.cpp
//Brief: A Column identifies a Drawable-associated property stored in a ctrl::Row.  
//TODO: Make Columns comparable so that I can one day have an interface that figures 
//      out what Columns different ColumnModels share.  
//Author: Andrew Olivier aolivier@ur.rochester.edu

//gl includes
#include "Data.cpp"

//c++ includes
#include <list>
#include <vector>
#include <memory>
#include <sstream>
#include <limits>

#ifndef CTRL_COLUMN_CPP
#define CTRL_COLUMN_CPP

namespace ctrl
{
  //A ColumnBase provides a way to hold a collection of objects that each know something about a specific type. 
  //ColumnBase also provides the user with a handle to the type of object in each column.  
  class ColumnBase
  { 
    public:
      ColumnBase(const std::string& name): fName(name), fPosition(std::numeric_limits<decltype(fPosition)>::max()) {}
      virtual ~ColumnBase() = default;
                                                                                                                     
      virtual std::unique_ptr<detail::DataBase> BuildData() const = 0;
                                                                                                                     
      void SetPosition(const size_t pos) { fPosition = pos; } //TODO: friend function of ColumnModel?
      inline size_t GetPosition() const { return fPosition; }

      inline std::string Name() const { return fName; }                                                                                                                     
    private: 
      std::string fName; //The name of this column when a TreeModel using it is drawn.
      size_t fPosition; //The position of this ColumnBase in the column model (note the lack of capitalization)
  };
                                                                                                                     
  //Concrete implementation of ColumnBase. Encodes the type information for the object it refers to so that 
  //Node can extract that information with a minimum of trouble.  
  template <class T>
  class Column: public ColumnBase
  {
    public:
     Column(const std::string& name): ColumnBase(name) {}
     virtual ~Column() = default;
                                                                                                                     
     virtual std::unique_ptr<detail::DataBase> BuildData() const override { return std::unique_ptr<detail::DataBase>(new detail::Data<T>()); }
  };

  //The list of columns in a crtl::Model.  Users should derive from this class to use it.  
  class ColumnModel
  {
    public:
      void Add(ColumnBase& col) //A ColumnBase must be Add()ed to ColumnModel to show up in trees that use a given model
      {
        col.SetPosition(fCols.size());
        fCols.push_back(&col);
      }

      //TODO: Set fPosition here somehow.  Class friendship?  Something more devious?  
      //      Already figured out I can't create the ColumnBases myself in this class 
      //      because the user needs access to the Column<T>s themselves for Node::operator[].
      inline size_t size() const { return fCols.size(); }
       
      std::vector<std::unique_ptr<detail::DataBase>> BuildData() const
      {
        std::vector<std::unique_ptr<detail::DataBase>> retVal;
        for(const auto& col: fCols) retVal.push_back(col->BuildData());
        return retVal;
      }

      inline std::string Name(const size_t col) const { return fCols[col]->Name(); }

    protected:
      ColumnModel() {} //Derive from this class to use it.  

    private:
      std::vector<ColumnBase*> fCols;
  };
}

#endif //CTRL_COLUMN_CPP
