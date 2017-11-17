#version 330 core

layout (lines_adjacency) in;
layout (triangle_strip, max_vertices = 4) out;

in VS_OUT
{
  vec4 color;
} vs_in[];

//uniform float width;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec4 userColor;

void MakeQuad(vec2 first, vec2 last, vec4 color)
{
  float width = 0.01; //TODO: Blend color between points
  userColor = color;
  //float theta = acos(dot(normalize(last-middle), normalize(middle-first)))/2.;
  //vec2 offset = width/2.*vec2(cos(theta), sin(theta));
  vec2 displacement = normalize(last-first);
  vec2 normal = vec2(-displacement.y, displacement.x)*width/2.;
  gl_Position = vec4(first-normal, 0., 1.);
  EmitVertex();

  gl_Position = vec4(first+normal, 0., 1.);
  EmitVertex();

  gl_Position = vec4(last-normal, 0., 1.);
  EmitVertex();

  gl_Position = vec4(last+normal, 0., 1.);
  EmitVertex();
}

void main()
{
  //MakeLine(gl_in[0].gl_Position.xy, gl_in[1].gl_Position.xy, gl_in[2].gl_Position.xy, vs_in[1].color);
  //MakeLine(gl_in[1].gl_Position.xy, gl_in[2].gl_Position.xy, gl_in[3].gl_Position.xy, vs_in[2].color);
  MakeQuad(gl_in[0].gl_Position.xy, gl_in[1].gl_Position.xy, vs_in[1].color);
  MakeQuad(gl_in[1].gl_Position.xy, gl_in[2].gl_Position.xy, vs_in[2].color);
  EndPrimitive();
}
