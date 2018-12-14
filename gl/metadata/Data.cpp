//File: Data.cpp
//Brief: A Data is a value that can be stringified for display to the user.  Internally, this is 
//       where I implement type erasure so that the user can have run-time tuples.  If you're not 
//       interested in how I've implemented type erasure, you probably don't really want to read 
//       this.  
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef CRTL_DETAIL_DATA_CPP
#define CRTL_DETAIL_DATA_CPP

//c++ includes
#include <string>
#include <sstream>

namespace ctrl
{
  namespace detail //Seriously, things are about to get ugly
  {
    //A DataBase holds a single element of string-convertible data
    class DataBase
    {
      public:
        virtual ~DataBase() = default;
                                                                                                                      
        virtual void* Get() = 0; //Type will be figured out by Column<T>'s T
        virtual std::string string() const = 0; //Turn whatever data is stored into a std::string
    };

    
    //Store actual data as a real type so I can delete it. 
    //Cast it to a void* and then back so that I can actually write a sane user interface.    
    template <class T>
    class Data: public DataBase
    {
      public:
        virtual ~Data() = default;
                                                                                                                      
        //Cast fData to void* so it can be cast back later.  
        virtual void* Get() override { return (void*)(&fData); }
            
        //Turn data into a string.  Using operator << instead of std::to_string() to support
        //VisID with minimal work.  
        virtual std::string string() const override 
        { 
          std::stringstream ss;
          ss << fData;
          return ss.str();
        }
                                                                                                                      
      protected:
        T fData; //The actual data stored in a DataBase
    };
  }
}

#endif //CRTL_DETAIL_DATA_CPP
