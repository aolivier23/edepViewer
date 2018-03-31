//File: Texture2D.cpp
//Brief: Convenience object that stores data for using an OpenGL texture.  
//Author: Andrew Olivier aolivier@ur.rochester.edu

//gl includes
#include "gl/objects/enumToType.h"

#ifndef MYGL_TEXTURE2D_CPP
#define MYGL_TEXTURE2D_CPP

namespace mygl
{
  template <class FORMAT>
  class Texture2D
  {
    public:
      //TODO: Use Image class template instead?
      Texture2D(const unsigned int width, const unsigned int height, FORMAT* data): Texture2D()
      {
        Use();
        //TODO: Other texture parameters?
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, enumToType<FORMAT>::Type, data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glBindTexture(GL_TEXTURE_2D, 0); //Unbind this texture
      }

      virtual ~Texture2D()
      {
        glDeleteTextures(1, &fTexID);
      }

      //Bind the texture as the current openGL texture
      void Use() { glBindTexture(GL_TEXTURE_2D, fTexID); } 

      //Read texture into user array data
      //TODO: It would be nice to take a type that knows about width and height and check that they agree with the size of this 
      //      texture.
      void ReadData(FORMAT* data) 
      {
        Use();
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, enumToType<FORMAT>::Type, data);

        glBindTexture(GL_TEXTURE_2D, 0); //Unbind this texture
      }      

      //Access to texture dimensions so user can do size checking
      unsigned int GetWidth() const { return fWidth; }
      unsigned int GetHeight() const { return fHeight; }

      //Attach to the current framebuffer
      void AttachCurrentFramebuffer(GLenum bufferMode = GL_FRAMEBUFFER)
      {
        Use();
        glFramebufferTexture2D(bufferMode, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fTexID, 0);

        glBindTexture(GL_TEXTURE_2D, 0); //Unbind this texture
      }

    private:
      unsigned int fTexID; //OpenGL's ID for this texture object
      unsigned int fWidth; //Width of this texture
      unsigned int fHeight; //Height of this texture
      //TODO: Other texture parameters as I need them
  };
}

#endif //MYGL_TEXTURE2D_H
