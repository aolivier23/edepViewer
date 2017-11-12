layout (lines_adjacency) in;
layout (triangle_strip, max_vertices = 4) out;

uniform float width;

void MakeLine(vec3 first, vec3 middle, vec3 last)
{
  float theta = acos(dot(normalize(last-middle), normalize(middle-first)))/2.;
  vec3 offset = width/2.*vec3(cos(theta), sin(theta));
  gl_Position = middle+offset;
  EmitVertex();

  gl_Position = middle-offset;
  EmitVertex();
}

void main()
{
  MakeLine(gl_in[0], gl_in[1], gl_in[2]);
  MakeLine(gl_in[1], gl_in[2], gl_in[3]);
  EndPrimitive();
}
