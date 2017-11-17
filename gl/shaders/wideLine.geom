#version 330 core

layout (lines_adjacency) in;
layout (triangle_strip, max_vertices = 4) out;

in VS_OUT
{
  vec4 color;
} vs_in[];

//uniform float width;

out vec4 userColor;

void MakeLine(vec2 first, vec2 middle, vec2 last, vec4 midColor)
{
  float width = 0.01;
  userColor = midColor;
  float theta = acos(dot(normalize(last-middle), normalize(middle-first)))/2.;
  vec2 offset = width/2.*vec2(cos(theta), sin(theta));
  gl_Position = vec4(middle+offset, 0., 1.);
  EmitVertex();

  gl_Position = vec4(middle-offset, 0., 1.);
  EmitVertex();
}

void main()
{
  MakeLine(gl_in[0].gl_Position.xy, gl_in[1].gl_Position.xy, gl_in[2].gl_Position.xy, vs_in[1].color);
  MakeLine(gl_in[1].gl_Position.xy, gl_in[2].gl_Position.xy, gl_in[3].gl_Position.xy, vs_in[2].color);
  EndPrimitive();
}
