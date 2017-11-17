#version 330 core

layout (lines_adjacency) in;
layout (triangle_strip, max_vertices = 4) out;

in VS_OUT
{
  vec4 color;
} vs_in[];

uniform float width;

out vec4 userColor;

void OffsetVertices(vec2 first, vec2 middle, vec2 last, vec4 color)
{
  userColor = color;
  //TODO: Use theta to prevent lines from showing gaps
  //TODO: Why does this algorithm produce ribbons?  
  //float theta = acos(dot(normalize(last-middle), normalize(middle-first)))/2.;
  //vec2 offset = width/2.*vec2(-sin(theta), cos(theta));
  vec2 dir = normalize(last-first);
  vec2 normal = vec2(-dir.y, dir.x)*width/2.;
  gl_Position = vec4(middle-normal, 0., 1.);
  EmitVertex();

  gl_Position = vec4(middle+normal, 0., 1.);
  EmitVertex();

  //gl_Position = vec4(last-offset, 0., 1.);
  //EmitVertex();

  //gl_Position = vec4(last+offset, 0., 1.);
  //EmitVertex();
}

void main()
{
  OffsetVertices(gl_in[0].gl_Position.xy, gl_in[1].gl_Position.xy, gl_in[2].gl_Position.xy, vs_in[1].color);
  OffsetVertices(gl_in[1].gl_Position.xy, gl_in[2].gl_Position.xy, gl_in[3].gl_Position.xy, vs_in[2].color);
  EndPrimitive();
}
