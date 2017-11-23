#version 330 core

layout (lines_adjacency) in;
layout (triangle_strip, max_vertices = 12) out;

in VS_OUT
{
  vec4 color;
} vs_in[];

uniform float width;
uniform float borderWidth;
uniform vec4 borderColor;

out vec4 userColor;

void OffsetVertices(vec2 first, vec2 middle, vec2 last, vec4 color)
{
  userColor = color;
  vec2 dir = normalize(last-first);
  vec2 normal = vec2(-dir.y, dir.x)*width/2.;
  
  gl_Position = vec4(middle-normal, 0., 1.);
  EmitVertex();

  gl_Position = vec4(middle+normal, 0., 1.);
  EmitVertex();
}

void TopBorderVertices(vec2 first, vec2 middle, vec2 last)
{
  userColor = borderColor;
  vec2 dir = normalize(last-first);
  vec2 normal = vec2(-dir.y, dir.x)*width/2.;
  vec2 border = vec2(-dir.y, dir.x)*borderWidth/2.;
  gl_Position = vec4(middle+normal, 0., 1.);
  EmitVertex();

  gl_Position = vec4(middle+normal+borderWidth/2., 0., 1.);
  EmitVertex();
}

void BottomBorderVertices(vec2 first, vec2 middle, vec2 last)
{
  userColor = borderColor;
  vec2 dir = normalize(last-first);
  vec2 normal = vec2(-dir.y, dir.x)*width/2.;
  vec2 border = vec2(-dir.y, dir.x)*borderWidth/2.;
  gl_Position = vec4(middle-normal, 0., 1.);
  EmitVertex();

  gl_Position = vec4(middle-normal-borderWidth/2., 0., 1.);
  EmitVertex();
}

void main()
{
  OffsetVertices(gl_in[0].gl_Position.xy, gl_in[1].gl_Position.xy, gl_in[2].gl_Position.xy, vs_in[1].color);
  OffsetVertices(gl_in[1].gl_Position.xy, gl_in[2].gl_Position.xy, gl_in[3].gl_Position.xy, vs_in[2].color);
  EndPrimitive();
  
  TopBorderVertices(gl_in[0].gl_Position.xy, gl_in[1].gl_Position.xy, gl_in[2].gl_Position.xy);
  TopBorderVertices(gl_in[1].gl_Position.xy, gl_in[2].gl_Position.xy, gl_in[3].gl_Position.xy);
  EndPrimitive();

  BottomBorderVertices(gl_in[0].gl_Position.xy, gl_in[1].gl_Position.xy, gl_in[2].gl_Position.xy);
  BottomBorderVertices(gl_in[1].gl_Position.xy, gl_in[2].gl_Position.xy, gl_in[3].gl_Position.xy);
  EndPrimitive();
}
