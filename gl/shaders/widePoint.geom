#version 330 core

layout (points) in;
layout (triangle_strip, max_vertices = 20) out;

in VS_OUT
{
  vec4 color;
} vs_in[];

uniform float radius;

out vec4 userColor;

//TODO: Get projection matrix so that vertex size shrinks as I zoom out?

void EmitNGon(vec2 center, vec4 color, int nPoints)
{
  userColor = color;
  const float pi = 3.1415926535897932384626433832;
  float nPointsFloat = float(nPoints);
  float deltaAngle = 2*pi/nPointsFloat;

  //Handle first 3 points specially because they form only one Primitive
  //TODO: No longer necessary.  
  gl_Position = vec4(center+vec2(radius, 0.), 0., 1.);
  EmitVertex();

  gl_Position = vec4(center+radius*vec2(cos(deltaAngle), sin(deltaAngle)), 0., 1.);
  EmitVertex();

  gl_Position = vec4(center+radius*vec2(cos(deltaAngle), -sin(deltaAngle)), 0., 1.);
  EmitVertex();

  for(float angle = 2*deltaAngle; angle < pi; angle += deltaAngle)
  {
    gl_Position = vec4(center+radius*vec2(cos(angle), sin(angle)), 0., 1.);
    EmitVertex(); 

    gl_Position = vec4(center+radius*vec2(cos(angle), -sin(angle)), 0., 1.);
    EmitVertex();
  }

  //Last 1 OR two triangles depending on whether nPoints is even.  
  //Writing the loop this way removes an if statement.
  if(nPoints > 3) //A triangle does not need 5 points!
  {
    gl_Position = vec4(center+vec2(-radius, 0.), 0., 1.);
    EmitVertex();

    if(nPoints%2 == 1)
    {
      gl_Position = vec4(center+vec2(-radius, 0.), 0., 1.);
      EmitVertex();
    }
  }
}

void main()
{
  //TODO: determine nPoints based on radius
  EmitNGon(gl_in[0].gl_Position.xy, vs_in[0].color, 10);
  EndPrimitive();
}
