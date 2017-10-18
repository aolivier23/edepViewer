#version 330 core
//in vec3 userColor;
in vec2 userTex;

out vec4 color;

uniform sampler2D texture0;
uniform sampler2D texture1;

void main()
{
  color = mix(texture(texture0, userTex), texture(texture1, userTex), 0.2);
}
