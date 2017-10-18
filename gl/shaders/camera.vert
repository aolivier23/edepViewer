#version 330 core
layout (location=0) in vec3 pos;
//layout (location=1) in vec3 color;
layout (location=1) in vec2 tex;

out vec3 userColor;
out vec2 userTex;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
  gl_Position = projection*view*model*vec4(pos, 1.0f);
  //userColor = color;
  userTex = vec2(tex.x, 1.0f-tex.y);
}
