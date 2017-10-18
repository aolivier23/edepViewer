#version 330 core

out vec4 color;

uniform vec4 userColor; //Receive this from the user

void main()
{
  color = userColor;
}
