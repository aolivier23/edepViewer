#version 330 core

layout (points) in;
layout (triangle_strip, max_vertices = 32) out;

in VS_OUT
{
  vec4 color;
} vs_in[];

uniform float radius;
uniform float borderWidth;
uniform vec4 borderColor;

const int nPoints = 10;

out vec4 userColor;

//TODO: Get projection matrix so that vertex size shrinks as I zoom out?

void EmitNGon(vec2 center, vec4 color)
{
  userColor = color;
  const float pi = 3.1415926535897932384626433832;
  float nPointsFloat = float(nPoints);
  float deltaAngle = 2*pi/nPointsFloat;

  //Handle first 3 points specially because they form only one Primitive
  //TODO: No longer necessary.  
  gl_Position = vec4(center+vec2(radius, 0.), 0., 1.);
  EmitVertex();

  for(float angle = deltaAngle; angle < pi; angle += deltaAngle)
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

void EmitBorder(vec2 center)
{
  userColor = borderColor;
  const float pi = 3.1415926535897932384626433832;
  float nPointsFloat = float(nPoints);
  float deltaAngle = 2*pi/nPointsFloat;

  for(float angle = 0; angle < 2*pi; angle += deltaAngle)
  {
    gl_Position = vec4(center+radius*vec2(cos(angle), sin(angle)), 0., 1.);
    EmitVertex();

    gl_Position = vec4(center+(radius+borderWidth)*vec2(cos(angle), sin(angle)), 0., 1.);
    EmitVertex();
  }

  gl_Position = vec4(center+radius*vec2(1., 0.), 0., 1.);
  EmitVertex();

  gl_Position = vec4(center+(radius+borderWidth)*vec2(1., 0.), 0., 1.);
  EmitVertex();
}

void main()
{
  //TODO: determine nPoints based on radius
  EmitNGon(gl_in[0].gl_Position.xy, vs_in[0].color);
  EndPrimitive();

  EmitBorder(gl_in[0].gl_Position.xy);
  EndPrimitive();
}
