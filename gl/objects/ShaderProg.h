//File: ShaderProg.h
//Brief: Encapsulates creating an OpenGL shader program from two files.  
//Author: Andrew Olivier aolivier@ur.rochester.edu
//Date: 4/14/2017

#ifndef GL_SHADERPROG_H
#define GL_SHADERPROG_H

//glad includes
#include "glad/include/glad/glad.h"

//glm include
#include <glm/gtc/matrix_transform.hpp> //for glm::mat4, which is a typedef

//c++ includes
#include <string>

namespace mygl
{
  class ShaderProg
  {
    public: 
      ShaderProg(const std::string& fragName, const std::string& vertName); //Create a shader program with a fragment shader and a 
                                                                            //vertex shader.
      ShaderProg(const std::string& fragStr, const std::string& vertStr, const std::string& geomStr); //Create a shader program 
                                                                                                      //with a framgment shader, 
                                                                                                      //a vertex shader, and a 
                                                                                                      //geometry shader.
      
      void Use(); //Make this the active program

      //Function overloads to set uniforms of various types
      //Note that all SetUniform methods make this shader the current program because 
      //glUniform's documentation claims that it only works for the current program
      void SetUniform(const std::string& name, const float value);
      void SetUniform(const std::string& name, const int value); 
      void SetUniform(const std::string& name, const float first, const float second);
      void SetUniform(const std::string& name, const glm::vec2& vec);
      void SetUniform(const std::string& name, const float first, const float second, const float third);
      void SetUniform(const std::string& name, const glm::vec3& vec);
      void SetUniform(const std::string& name, const float first, const float second, const float third, const float fourth);
      void SetUniform(const std::string& name, const glm::vec4& vec);
      void SetUniform(const std::string& name, const glm::mat4& mat);

      //Provide user access to program ID.  
      //TODO: Is there a better way to encapsulate operations that need this?  Yes, it is provided by the SetUniform() methods
      //GLuint GetID() const;
      
    private:
      GLuint fProgID; //OpenGL's ID for this program

      GLuint MakeShader(const std::string& fileName, GLenum shader_type); //Compile a shader to be ready for program linking.  
                                                                          //Returns the openGL ID number for the shader

      void CheckSuccess() const;
      void ValidateVert(const std::string& vertName) const;
      void ValidateFrag(const std::string& fragName) const;
      void ValidateGeom(const std::string& geomName) const; 
  };
}

#endif
