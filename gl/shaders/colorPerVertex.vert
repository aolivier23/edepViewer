#version 330 core
layout (location=0) in vec3 pos;
layout (location=1) in vec4 color;

out vec4 userColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out VS_OUT
{
  vec4 color;
} vs_out;

void main()
{
  gl_Position = projection*view*model*vec4(pos, 1.0f);
  userColor = color;
  vs_out.color = color;
}
