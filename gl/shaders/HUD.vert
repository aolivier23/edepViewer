#version 330 core
layout (location=0) in vec3 pos;

out vec4 userColor;

uniform mat4 model;
//uniform mat4 view; //The point of this shader is to completely ignore the view matrix
uniform mat4 projection;

void main()
{
  gl_Position = projection*model*vec4(pos, 1.0f);
}
