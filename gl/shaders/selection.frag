#version 330 core
uniform vec4 idColor;

out vec4 color;

void main()
{
  color = idColor; //Provided by the user.  The user should be able to map this color back 
                   //to a drawn object.  
}
