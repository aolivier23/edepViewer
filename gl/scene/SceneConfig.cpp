//File: SceneConfig.cpp
//Brief: Base class for Scene configuration.  Gives the user a place to make OpenGL calls for a given Scene each time that Scene is 
//       about to Render.  Derive from this class in your drawing plugin to use it.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//GLAD includes to get OpenGL functions
#include "glad/include/glad/glad.h"

#ifndef MYGL_SCENECONFIG_CPP
#define MYGL_SCENECONFIG_CPP

namespace mygl
{
  class SceneConfig
  {
    public:
      SceneConfig() = default;
      virtual ~SceneConfig() = default;

      //Override these functions and configure your Scene here!
      virtual void BeforeRender() 
      {
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
      }

      virtual void AfterRender() 
      {
        glDisable(GL_DEPTH_TEST);
      }
  };
}

#endif
