#version 330 core
layout (location=0) in vec3 pos;
layout (location=1) in vec4 color;

uniform mat4 model;
//uniform mat4 view; //The point of this shader is to completely ignore the view matrix
uniform mat4 projection;

out VS_OUT
{
  vec4 color;
} vs_out;

void main()
{
  gl_Position = projection*model*vec4(pos, 1.0f);
  vs_out.color = color; 
}
