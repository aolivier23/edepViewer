//File: GenException.cpp
//Brief: Implements a general-purpose exception class.  Somewhat inspired by cet::exception.
//Author: Andrew Olivier aolivier@ur.rochester.edu
//Date: 4/14/2017

//Local includes
#include "GenException.h" //The base class

namespace util
{
  GenException::GenException(const std::string& cat) noexcept
  {
    fWhat = cat+":\n";
  }
  
  GenException::GenException(const GenException& e) noexcept
  {
    fWhat = e.fWhat;
  }
  
  GenException& GenException::operator=(const GenException& e) noexcept
  {
    fWhat = e.fWhat;
    return *this;
  }
  
  const char* GenException::what() const noexcept
  {
    return fWhat.c_str();
  }
}
