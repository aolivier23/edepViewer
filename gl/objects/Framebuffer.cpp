//File: Framebuffer.cpp
//Brief: A simple representation of OpenGL's framebuffer object.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//glad includes
#include "glad/include/glad/glad.h"

//local includes
#include "gl/objects/Framebuffer.h"

namespace mygl
{
  Framebuffer::Framebuffer()
  {
    glGenFramebuffers(1, &fBufferID);
  }

  Framebuffer::~Framebuffer()
  {
    glDeleteFramebuffers(1, &fBufferID);
  }

  void Framebuffer::Use()
  {
    glBindFramebuffer(GL_FRAMEBUFFER, fBufferID);
  }

  void Framebuffer::Default()
  {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }
} 
