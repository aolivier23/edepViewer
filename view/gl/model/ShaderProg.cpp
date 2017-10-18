//File: ShaderProg.cpp
//Brief: Implements an OpenGL shader program class
//Author: Andrew Olivier aolivier@ur.rochester.edu
//Date: 4/14/2017

//local includes
#include "ShaderProg.h" //Header
#include "GenException.h" //General-purpose exception class

//glm includes
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

//c++ includes
#include <fstream> //For reading from files
#include <iostream> //For printing a warning

namespace view
{
  ShaderProg::ShaderProg(const std::string& vertStr, const std::string& fragStr)
  {
    //Check for passing fragment shader source for vertex shader
    //You are welcome to add more conventions here!
    if(vertStr.find(".frag") != std::string::npos) 
    {
      std::cerr << "WARNING in ShaderProg Constructor: "
                << "You may have passed the source code "
                << "for a fragment shader as the vertex " 
                << "shader source.  If this is the case, "
                << "you will almost certainly get many shader "
                << "compiler errors below.\n";
    }

    //Check for passing vertex shader source for fragment shader
    //You are welcome to add more conventions here!
    if(fragStr.find(".vert") != std::string::npos) 
    {
      std::cerr << "WARNING in ShaderProg Constructor: "
                << "You may have passed the source code "
                << "for a vertex shader as the fragment "                         
                << "shader source.  If this is the case, "
                << "you will almost certainly get many shader "
                << "compiler errors below.\n";
    }

    //Compile the shaders
    auto vert = MakeShader(vertStr, GL_VERTEX_SHADER);
    auto frag = MakeShader(fragStr, GL_FRAGMENT_SHADER);

    //Make a shader program from the shaders just compiled
    fProgID = glCreateProgram();
    glAttachShader(fProgID, vert);
    glAttachShader(fProgID, frag);
    glLinkProgram(fProgID);
  
    //Delete shaders once they have been used
    glDeleteShader(vert);
    glDeleteShader(frag);
  
    //Check whether shader program linking failed
    GLint success;
    GLchar infoLog[512];
    glGetProgramiv(fProgID, GL_LINK_STATUS, &success);
    if(!success)
    {
      glGetProgramInfoLog(fProgID, 512, nullptr, infoLog);
      throw util::GenException("Shader Program Link Error") << infoLog << "\n";
    }
  }

  GLuint ShaderProg::MakeShader(const std::string& fileName, const GLenum shader_type)
  {
    //First, get the source code for this shader
    std::ifstream sourceFile(fileName);
    if(!sourceFile.is_open()) throw util::GenException("Source File Not Found") 
                                    << "Failed to find source code file " << fileName << "\n";
    
    //Next, get the source code into a const GLchar* for openGL to read
    std::string line = "", sourceStr = "";
    while(!sourceFile.eof())
    {
      std::getline(sourceFile, line);
      sourceStr += line+"\n";
    }
    sourceFile.close();
  
    const GLchar* sourceChar = sourceStr.c_str();

    //Attempt to compile the shader source code
    GLuint shader = glCreateShader(shader_type);
    glShaderSource(shader, 1, &sourceChar, nullptr);
    glCompileShader(shader);
  
    //Check that the vertex shader compiled
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
      glGetShaderInfoLog(shader, 512, nullptr, infoLog);
      throw util::GenException("Shader Compile Error") << " In shader source file " << fileName  << ":\n" 
                                                 << infoLog 
                                                 << "\nSource code was:\n" << sourceChar << "\n";
    }

    //Return openGL's identifier for this shader
    return shader;
  }

  void ShaderProg::Use()
  {
    glUseProgram(fProgID);
  }

  void ShaderProg::SetUniform(const std::string& name, const int value)
  {
    Use();
    auto location = glGetUniformLocation(fProgID, name.c_str());
    glUniform1i(location, value);
  }

  void ShaderProg::SetUniform(const std::string& name, const float value)
  {
    Use();
    auto location = glGetUniformLocation(fProgID, name.c_str());
    glUniform1f(location, value);
  }

  void ShaderProg::SetUniform(const std::string& name, const float first, const float second)
  {
    Use();
    auto location = glGetUniformLocation(fProgID, name.c_str());
    glUniform2f(location, first, second);
  }

  void ShaderProg::SetUniform(const std::string& name, const float first, const float second, const float third)
  {
    Use();
    auto location = glGetUniformLocation(fProgID, name.c_str());
    glUniform3f(location, first, second, third);
  }

  void ShaderProg::SetUniform(const std::string& name, const float first, const float second, const float third, const float fourth)
  {
    Use();
    auto location = glGetUniformLocation(fProgID, name.c_str());
    glUniform4f(location, first, second, third, fourth);
  }

  void ShaderProg::SetUniform(const std::string& name, const glm::mat4& mat)
  {
    Use();
    auto location = glGetUniformLocation(fProgID, name.c_str());
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(mat));
  }

}
