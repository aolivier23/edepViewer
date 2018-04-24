//File: enumToType.h
//Brief: Template specializations for opengl variable type enums.  Useful for things like glTexImage2D
//       Provides specializations for most of the types at https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glTexImage2D.xhtml
//Author: Andrew Olivier aolivier@ur.rochester.edu

//glad includes
#include "glad/include/glad/glad.h"

//c++ includes
#include <type_traits>

#ifndef MYGL_ENUMTOTYPE_H
#define MYGL_ENUMTOTYPE_H

namespace mygl
{
  //Base definition
  //TODO: It would be nice if this crashed the build somehow
  template <class FORMAT>
  struct enumToType
  {
    std::enable_if<false, decltype(GL_INVALID_ENUM)> Type = GL_INVALID_ENUM; //Not what this enum is really for, but gets the point 
                                                                             //across and should have the intended result
  };

  template <>
  struct enumToType<unsigned char>
  {
    GLenum Type = GL_UNSIGNED_BYTE;
  };

  template <>
  struct enumToType<unsigned int>
  {
    GLenum Type = GL_UNSIGNED_INT;
  };

  template <>
  struct enumToType<float>
  {
    GLenum Type = GL_FLOAT;
  };

  template <>
  struct enumToType<char>
  {
    GLenum Type = GL_BYTE;
  };

  template <>
  struct enumToType<unsigned short>
  {
    GLenum Type = GL_UNSIGNED_SHORT;
  };

  template <>
  struct enumToType<short>
  {
    GLenum Type = GL_SHORT;
  };

  template <>
  struct enumToType<int>
  {
    GLenum Type = GL_INT;
  };

  //TODO: The reference above lists lots of other types whose mapping to built-in c++ types is not clear to me.  
}

#endif //MYGL_ENUMTOTYPE_H
