//File: ShaderProg.cpp
//Brief: Implements an OpenGL shader program class
//Author: Andrew Olivier aolivier@ur.rochester.edu
//Date: 4/14/2017

//local includes
#include "gl/objects/ShaderProg.h" //Header

//util includes
#include "util/GenException.h" //General-purpose exception class

//glm includes
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

//c++ includes
#include <fstream> //For reading from files
#include <iostream> //For printing a warning

namespace mygl
{
  ShaderProg::ShaderProg(const std::string& fragStr, const std::string& vertStr): fProgID(glCreateProgram())
  {
    //Make sure the user passed the right kinds of source code for the shaders
    ValidateVert(vertStr);
    ValidateFrag(fragStr);

    //Compile and attach the shaders
    const auto vert = MakeShader(vertStr, GL_VERTEX_SHADER);
    const auto frag = MakeShader(fragStr, GL_FRAGMENT_SHADER);
    glLinkProgram(fProgID);
  
    //Delete shaders once they have been used
    glDeleteShader(vert);
    glDeleteShader(frag);
  
    //Check whether shader program linking failed
    CheckSuccess();
  }

  ShaderProg::ShaderProg(const std::string& fragStr, const std::string& vertStr, const std::string& geomStr): fProgID(glCreateProgram())
  {
    //Make sure the user passed the right kinds of source code for the shaders
    ValidateVert(vertStr);
    ValidateFrag(fragStr);
    ValidateGeom(geomStr);

    //Compile and attach the shaders
    const auto vert = MakeShader(vertStr, GL_VERTEX_SHADER);
    const auto frag = MakeShader(fragStr, GL_FRAGMENT_SHADER);
    const auto geom = MakeShader(geomStr, GL_GEOMETRY_SHADER);
    glLinkProgram(fProgID);

    //Delete shaders once they have been used
    glDeleteShader(vert);
    glDeleteShader(frag);
    glDeleteShader(geom);

    //Check whether shader program linking failed
    CheckSuccess();
  }

  void ShaderProg::CheckSuccess() const
  {
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

  void ShaderProg::ValidateVert(const std::string& vertName) const 
  {
    //Check for passing fragment shader source for vertex shader
    //You are welcome to add more conventions here!
    if(vertName.find(".vert") == std::string::npos)
    {
      std::cerr << "WARNING in ShaderProg Constructor: "
                << "Passed a shader program in a file "
                << "named " << vertName << " as the "
                << "vertex shader.  However, the program "
                << "name doesn't have .vert in it.  You "
                << "may have passed the wrong source file "
                << "for the vertex shader!\n";
    }
  }
  
  void ShaderProg::ValidateFrag(const std::string& fragName) const
  {
    //Check for passing fragment shader source for vertex shader
    //You are welcome to add more conventions here!
    if(fragName.find(".frag") == std::string::npos)
    {
      std::cerr << "WARNING in ShaderProg Constructor: "
                << "Passed a shader program in a file "
                << "named " << fragName << " as the "
                << "fragment shader.  However, the program "
                << "name doesn't have .frag in it.  You "
                << "may have passed the wrong source file "
                << "for the vertex shader!\n";
    }
  }

  void ShaderProg::ValidateGeom(const std::string& geomName) const
  {
    //Check for passing fragment shader source for vertex shader
    //You are welcome to add more conventions here!
    if(geomName.find(".geom") == std::string::npos)
    {
      std::cerr << "WARNING in ShaderProg Constructor: "
                << "Passed a shader program in a file "
                << "named " << geomName << " as the "
                << "geometry shader.  However, the program "
                << "name doesn't have .geom in it.  You "
                << "may have passed the wrong source file "
                << "for the vertex shader!\n";
    }
  }

  GLuint ShaderProg::MakeShader(const std::string& fileName, const GLenum shader_type)
  {
    //It is assumed that fProgID has already been set before this function is called.
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

    //Attach this shader to the current shader program
    glAttachShader(fProgID, shader);

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

  void ShaderProg::SetUniform(const std::string& name, const glm::vec2& vec)
  {
    SetUniform(name, vec.x, vec.y);
  }

  void ShaderProg::SetUniform(const std::string& name, const float first, const float second, const float third)
  {
    Use();
    auto location = glGetUniformLocation(fProgID, name.c_str());
    glUniform3f(location, first, second, third);
  }

  void ShaderProg::SetUniform(const std::string& name, const glm::vec3& vec)
  {
    SetUniform(name, vec.x, vec.y, vec.z);
  }

  void ShaderProg::SetUniform(const std::string& name, const float first, const float second, const float third, const float fourth)
  {
    Use();
    auto location = glGetUniformLocation(fProgID, name.c_str());
    glUniform4f(location, first, second, third, fourth);
  }

  void ShaderProg::SetUniform(const std::string& name, const glm::vec4& vec)
  {
    SetUniform(name, vec.r, vec.g, vec.b, vec.a);
  }

  void ShaderProg::SetUniform(const std::string& name, const glm::mat4& mat)
  {
    Use();
    auto location = glGetUniformLocation(fProgID, name.c_str());
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(mat));
  }

}
