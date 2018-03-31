//File: VAO.cpp
//Brief: A collection of OpenGL resources for specifying vertices to be drawn in a Scene.  See the header for a 
//       detailed description of my convoluted object model. 
//Author: Andrew Olivier aolivier@ur.rochester.edu

//glad includes
#include "glad/include/glad/glad.h" //For OpenGL functions and enums

//glm includes
#include <glm/glm.hpp>

//model includes
#include "gl/model/Drawable.h"

namespace mygl
{
  //Get handles to OpenGL resources that I will upload later.
  VAO::VAO()
  {
    glGenVertexArrays(1, &fVAO);
    glBindVertexArray(fVAO);

    glGenBuffers(1, &fEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, fEBO);

    glGenBuffers(1, &fVBO);
    glBindBuffer(GL_ARRAY_BUFFER, fVBO);

    //Map the definition of Drawable::Vertex to an organization of data on the GPU
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(0));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(sizeof(glm::vec3)));

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
  unsigned int VAO::Register(const std::vector<Vertex>& vertices)
  { 
    const auto result = fVertices.size();
    fVertices.insert(vertices.begin(), vertices.end());
    return result;
  }

  unsigned int VAO::Register(const std::vector<Vertex>& vertices, const std::vector<unsigned int> indices)
  {
    const auto result = fIndices.size();
    fVertices.insert(vertices.begin(), vertices.end());
    fIndices.insert(indices.begin(), indices.end());
    return result;
  }

  void VAO::Use()
  {
    if(fUpload) //Upload data to the GPU once ever?
    {
      UploadData();
      fUpload = false;
    }

    glBindVertexArray(fVAO);
  }

  //Send managed data to the GPU
  void VAO::Upload()
  {
    glBindVertexArray(fVAO);

    //Send indices to the GPU
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, fEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, fIndices.size()*sizeof(GLuint), &fIndices[0], GL_STATIC_DRAW);

    //Send vertices to the GPU
    glBindBuffer(GL_ARRAY_BUFFER, fVBO);
    glBufferData(GL_ARRAY_BUFFER, fVertices.size()*sizeof(Vertex), &fVertices[0], GL_STATIC_DRAW);

    glBindVertexArray(0); //Put current vertex array in unbound state to detect error more easily

    //Free up memory that is not used anymore?  If I plan to call Upload() multiple times with the same data, don't do this!
    fVertices.clear();
    fIndices.clear(); 
  }
} 
