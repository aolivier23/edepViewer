//File: Framebuffer.h
//Brief: Simple parameters for an OpenGL framebuffer object.  Might need to get more complicated in the future.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef MYGL_FRAMEBUFFER_H
#define MYGL_FRAMEBUFFER_H

namespace mygl
{
  class Framebuffer
  {
    public:
      Framebuffer();
      virtual ~Framebuffer();

      void Use(); //Make this the current Framebuffer
      static void Default(); //Make the default framebuffer the current framebuffer
     
    private:
      unsigned int fBufferID; //The OpenGL ID for this object's framebuffer object
  };
}

#endif //MYGL_FRAMEBUFFER_H
