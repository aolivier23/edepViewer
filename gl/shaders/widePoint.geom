#version 330 core

layout (points) in;
layout (triangle_strip, max_vertices = 10) out;

in VS_OUT
{
  vec4 color;
} vs_in[];

uniform float radius;

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

void EmitNGon(vec2 center, vec4 color, unsigned int nPoints)
{
  const float pi = 180.;
  const float deltaAngle = nPoints/pi;
  //Handle first 3 points specially because they form only one Primitive
  gl_Position = vec4(center+vec2(0., radius), 0., 1.);
  EmitVertex();

  gl_Position = vec4(center+radius*vec2(cos(deltaAngle), sin(deltaAngle)), 0., 1.);
  EmitVertex();

  gl_Position = vec4(center+radius*vec2(cos(deltaAngle), -sin(deltaAngle)), 0., 1.);
  EmitVertex();
  EmitPrimitive();

  for(float angle = 2*deltaAngle; angle < pi; angle += deltaAngle)
  {
    gl_Position = vec4(center+radius*vec2(cos(angle), sin(angle)), 0., 1.);
    EmitVertex(); 
    EmitPrimitive();

    gl_Position = vec4(center+radius*vec2(cos(angle), -sin(angle)), 0., 1.);
    EmitVertex();
    EmitPrimitive();
  }

  //Last 1 OR two triangles depending on whether nPoints is even.  
  //Writing the loop this way removes an if statement.
  if(nPoints > 3) //A triangle does not need 5 points!
  {
    gl_Position = vec4(center+vec2(-radius, 0.), 0., 1.);
    EmitVertex();
    EmitPrimitive();

    if(nPoints%2 == 1)
    {
      gl_Position = vec4(center+vec2(-radius, 0.), 0., 1.);
      EmitVertex();
      EmitPrimitive();
    }
  }
}

void main()
{
  //TODO: determine nPoints based on radius
  EmitNGon(gl_in[0].gl_Position.xy, vs_in[0].color, 5);
}
