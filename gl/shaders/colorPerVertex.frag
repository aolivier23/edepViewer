#version 330 core
//in vec3 userColor;
in vec4 userColor;

out vec4 color;

void main()
{
  color = userColor; //Forwarded from the vertex shader
}
