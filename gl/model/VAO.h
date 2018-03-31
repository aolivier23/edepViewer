//File: VAO.h
//Brief: A shared collection of vertex information for multiple objects.  c++ wrapper over an OpenGL Vertex Array Object.  
//       OpenGL VAO objects consist of a group of buffers that can together be sent to the GPU and made current for rendering.
//       So far, I have been using VAOs to store vertices and sometimes indices.  Model for drawing with this VAO class goes as:
//
//1.) User creates a Scene.  Each Scene has a VAO that can hold Vertex data and index data.
//2.) User requests Drawables from a Scene.  Each Drawable is passed a VAO in its' constructor.
//3.) In the constructor, each Drawable gives its' vertices and indices, if any, to the VAO, and 
//    the VAO returns the position of the first vertex in that Drawable.  The Drawable should 
//    keep track of the number of vertices it has to figure out the range of vertices to use.
//4.) When a Scene is asked to Render(), it binds its' VAO.  This makes the vertices and indices for all objects 
//    in that Scene available during Render().
//5.) A Scene tells some Drawables to Draw().  This dispatches to a subclass's DoDraw() which requests vertex data from its' 
//    first vertex (stored from the VAO earlier) to first vertex plus number of vertices.  
//6.) When a Scene has finished Render()ing, it unbinds its' VAO.  
//7.) When a Scene is Clear()ed or destroyed, its' VAO is destroyed implicitly, and this deallocates the GPU resources 
//    (OpenGL VAO, vertex buffer, and index buffer) managed by that VAO. 
//Author: Andrew Olivier aolivier@ur.rochester.edu

//model includes
#include "gl/model/Drawable.h"

//c++ includes
#include <vector>

#ifndef MYGL_VAO_H
#define MYGL_VAO_H

namespace mygl
{
class VAO
  {
    public:
      VAO(); //Allocate OpenGL resources, but no vertices to bind yet.
      virtual ~VAO(); //Deallocate all OpenGL resources managed

      //Interfaces between Drawables and a VAO
      unsigned int Register(const std::vector<Drawable::Vertex>& vertices); //Register vertices only to be used with glDrawArrays()
      unsigned int Register(const std::vector<Drawable::Vertex>& vertices, const std::vector<unsigned int>& indices); //Register vertices and indices to 
                                                                                                            //be used with glDrawElements()

      void Use(); //Bind managed OpenGL resources for use during drawing.  The first time this is called, send OpenGL resources to the GPU.

    private:
      //"Names" of OpenGL resources managed
      unsigned int fVAO; //The index of the OpenGL VAO managed
      unsigned int fVBO; //The index of the OpenGL Vertex Buffer Object (VBO) managed
      unsigned int fEBO; //The index of the OpenGL Element Buffer Object (EBO) of indices managed

      //Data from Drawables that needs to be sent to the GPU.  It will all be sent at once, then deleted from here
      std::vector<Drawable::Vertex> fVertices; //Vertices
      std::vector<unsigned int> fIndices; //Indices

      //Internal flag since structure is weird
      bool fUploadData; //Should I upload data to the GPU?

      //Internal helper functions to encapsulate specific actions
      void Upload(); //Upload data to the GPU
  };
}

#endif //MYGL_VAO_H
