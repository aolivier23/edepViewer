//File: VAO.cpp
//Brief: A collection of OpenGL resources for specifying vertices to be drawn in a Scene.  See the header for a 
//       detailed description of my convoluted object model. 
//Author: Andrew Olivier aolivier@ur.rochester.edu

//model includes
#include "gl/model/VAO.h"

//glad includes
#include "glad/include/glad/glad.h" //For OpenGL functions and enums

//glm includes
#include <glm/glm.hpp>

namespace mygl
{
  //Get handles to OpenGL resources that I will upload later.
  VAO::VAO(): fUploadData(true)
  {
    glGenVertexArrays(1, &fVAO);
    glBindVertexArray(fVAO);

    glGenBuffers(1, &fEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, fEBO);

    glGenBuffers(1, &fVBO);
    glBindBuffer(GL_ARRAY_BUFFER, fVBO);

    //Map the definition of Drawable::Vertex to an organization of data on the GPU
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Drawable::Vertex), (GLvoid*)(0));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Drawable::Vertex), (GLvoid*)(sizeof(glm::vec3)));

    glBindVertexArray(0); //Put current vertex array in unbound state to detect error more easily
  }

  //Deallocate OpenGL resources
  VAO::~VAO() 
  {
    glDeleteVertexArrays(1, &fVAO);
    glDeleteBuffers(1, &fVBO);
    glDeleteBuffers(1, &fEBO);
  }

  //Interface between Drawables and VAO.  Return the offset to the first index for this Drawable.
  unsigned int VAO::Register(const std::vector<Drawable::Vertex>& vertices)
  { 
    const auto result = fVertices.size();
    fVertices.insert(fVertices.end(), vertices.begin(), vertices.end());
    return result;
  }

  unsigned int VAO::Register(const std::vector<Drawable::Vertex>& vertices, const std::vector<unsigned int>& indices)
  {
    const auto vertOffset = fVertices.size();
    const auto indOffset = fIndices.size();
    fVertices.insert(fVertices.end(), vertices.begin(), vertices.end());
    for(const auto& index: indices) fIndices.push_back(index + vertOffset);
    return indOffset;
  }

  void VAO::Use()
  {
    if(fUploadData) //Upload data to the GPU once ever?
    {
      Upload();
      fUploadData = false;
    }

    glBindVertexArray(fVAO);
  }

  //Send managed data to the GPU
  void VAO::Upload()
  {
    glBindVertexArray(fVAO);

    //Send indices to the GPU
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, fEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, fIndices.size()*sizeof(unsigned int), &fIndices[0], GL_STATIC_DRAW);

    //Send vertices to the GPU
    glBindBuffer(GL_ARRAY_BUFFER, fVBO);
    glBufferData(GL_ARRAY_BUFFER, fVertices.size()*sizeof(Drawable::Vertex), &fVertices[0], GL_STATIC_DRAW);

    glBindVertexArray(0); //Put current vertex array in unbound state to detect error more easily

    //Free up memory that is not used anymore?  If I plan to call Upload() multiple times with the same data, don't do this!
    fVertices.clear();
    fIndices.clear(); 
  }
} 
