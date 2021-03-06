//File: GenException.h
//Brief: Declaration of a general-use exception class with a category and an explanation.  
//       Inspired by cet::exception to some extent.
//Author: Andrew Olivier aolivier@ur.rochester.edu
//Date: 4/14/2017

//c++ includes
#include <exception> //For base class type
#include <string> //Because c-strings are awful
#include <sstream> //Because std::to_string cannot handle const char*

#ifndef GENEXCEPTION_H
#define GENEXCEPTION_H

namespace util
{
  class GenException: public std::exception
  {
    public:
      GenException(const std::string& cat) noexcept;
  
      GenException(const GenException& e) noexcept;
  
      virtual ~GenException() noexcept {};
  
      GenException& operator= (const GenException& e) noexcept;
  
      virtual const char* what() const noexcept;
  
      //For cet::exception-like syntax when throwing exceptions
      template <class T>
      GenException& operator<<(const T arg)
      {
        //TODO: Is there a better way to do this than with a temporary stringstream?
        //      I don't (think) I want to store the stringstream because I need fWhat to 
        //      exist when I return from what().
        //      std::to_string doesn't work with const char* (which is important in the use 
        //      case I designed this for).
        std::stringstream what_tmp;
        what_tmp << arg;
        fWhat += what_tmp.str(); 
        return *this;
      }
  
    protected:
      std::string fWhat; //Why this exception was thrown
                        //fWhat needs to be a std::string because the const char* I 
                        //return in what() would otherwise refer to a member of a temporary object!
  
    
  };
}

#endif
