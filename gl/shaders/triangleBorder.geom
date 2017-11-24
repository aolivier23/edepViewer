#version 330 core

layout (triangles_adjacency) in;
layout (triangle_strip, max_vertices = 12) out;

in VS_OUT
{
  vec4 color;
} vs_in[];

uniform float width;
uniform float borderWidth;
uniform vec4 borderColor;

out vec4 userColor;

bool isFront(vec3 first, vec3 second, vec3 third)
{
  return cross(second-first, third-first).z > 0.0;
}

bool IsEdge(vec3 first, vec3 second, vec3 third, vec3 other)
{
  return (isFront(first, second, third)) && (!isFront(first, third, other));
}

void EmitQuad(vec3 first, vec3 second)
{
  //TODO: Extend quad so that it matches the end of the next triangle's quad.  
  //      I think I could do this with the angle that the edges of this triangle make 
  //      at each vertex.
  userColor = borderColor;
  gl_Position = vec4(first, 1.0);
  EmitVertex();

  gl_Position = vec4(first+borderWidth*vec3(0.0, 1.0, 0.0), 1.0);
  EmitVertex();

  gl_Position = vec4(second, 1.0);
  EmitVertex();

  gl_Position = vec4(second+borderWidth*vec3(0.0, 1.0, 0.0), 1.0);
  EmitVertex();
}

void EmitEdgeIfFront(vec4 first, vec4 second, vec4 third, vec4 other)
{
  //TODO: Is it really a good idea to emit multiple primitives like this, or should I work more on 
  //      tesselating the shape I ultimately want to draw?  
  if(IsEdge(first.xyz, second.xyz, third.xyz, other.xyz)) EmitQuad(first.xyz, second.xyz);
  EndPrimitive();
}

void main()
{
  //Pass the original triangle through
  userColor = vs_in[0].color;
  gl_Position = gl_in[0].gl_Position;  
  EmitVertex();

  userColor = vs_in[2].color;
  gl_Position = gl_in[2].gl_Position;
  EmitVertex();

  userColor = vs_in[4].color;
  gl_Position = gl_in[4].gl_Position;
  EmitVertex();
  EndPrimitive();

  //Draw border if any edge of this triangle borders the back of the object drawn
  EmitEdgeIfFront(gl_in[0].gl_Position, gl_in[2].gl_Position, gl_in[4].gl_Position, gl_in[1].gl_Position);
  EmitEdgeIfFront(gl_in[2].gl_Position, gl_in[4].gl_Position, gl_in[0].gl_Position, gl_in[3].gl_Position);
  EmitEdgeIfFront(gl_in[4].gl_Position, gl_in[0].gl_Position, gl_in[2].gl_Position, gl_in[5].gl_Position);
}
