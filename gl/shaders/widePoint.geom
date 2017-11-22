#version 330 core

layout (points) in;
layout (triangle_strip, max_vertices = 10) out;

in VS_OUT
{
  vec4 color;
} vs_in[];

uniform float radius;

out vec4 userColor;

void EmitNGon(vec2 center, vec4 color, int nPoints)
{
  const float pi = 180.;
  float deltaAngle = nPoints/pi;
  //Handle first 3 points specially because they form only one Primitive
  gl_Position = vec4(center+vec2(0., radius), 0., 1.);
  EmitVertex();

  gl_Position = vec4(center+radius*vec2(cos(deltaAngle), sin(deltaAngle)), 0., 1.);
  EmitVertex();

  gl_Position = vec4(center+radius*vec2(cos(deltaAngle), -sin(deltaAngle)), 0., 1.);
  EmitVertex();
  EndPrimitive();

  for(float angle = 2*deltaAngle; angle < pi; angle += deltaAngle)
  {
    gl_Position = vec4(center+radius*vec2(cos(angle), sin(angle)), 0., 1.);
    EmitVertex(); 
    EndPrimitive();

    gl_Position = vec4(center+radius*vec2(cos(angle), -sin(angle)), 0., 1.);
    EmitVertex();
    EndPrimitive();
  }

  //Last 1 OR two triangles depending on whether nPoints is even.  
  //Writing the loop this way removes an if statement.
  if(nPoints > 3) //A triangle does not need 5 points!
  {
    gl_Position = vec4(center+vec2(-radius, 0.), 0., 1.);
    EmitVertex();
    EndPrimitive();

    if(nPoints%2 == 1)
    {
      gl_Position = vec4(center+vec2(-radius, 0.), 0., 1.);
      EmitVertex();
      EndPrimitive();
    }
  }
}

void main()
{
  //TODO: determine nPoints based on radius
  EmitNGon(gl_in[0].gl_Position.xy, vs_in[0].color, 10);
}
