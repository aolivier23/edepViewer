//File: PolyMesh.h
//Brief: A PolyMesh is a collection of vertices, with an overall color, to be drawn using opengl.  The vertices in a 
//       PolyMesh are used to create polygons that form the face of the mesh.  PolyMesh is designed to 
//       draw individual TGeoShapes from CERN's ROOT framework.  
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef MYGL_POLYMESH_H
#define MYGL_POLYMESH_H

//c++ includes
#include <vector>

//glm includes
#include <glm/glm.hpp>

//gl includes
#include "gl/model/Drawable.h"
#include "gl/model/ShaderProg.h"

class TGeoVolume;

namespace mygl
{
  class PolyMesh: public Drawable
  {
    public:
      //Note: I have to implement template members in the class header
      template <class CONTAINER, class INDICES> //CONTAINER is any container with begin() and end() members
                                                //INDICES is any container with begin() and end() members 
                                                //whose elements ALSO have begin() and end() members
      PolyMesh(const glm::mat4& model, const CONTAINER& vertices, const INDICES& indices, 
               const glm::vec4& color): Drawable(model), fNVertices(), fIndexOffsets()
      {
        Init(vertices, indices, color);
      }

      PolyMesh(const glm::mat4& model, TGeoVolume* vol, const glm::vec4& color);

      //PolyMesh(const std::initializer_list<glm::vec3>&& list, const glm::vec3& color); //Removing support for this because of how 
                                                                                         //complicated it would become.

      virtual ~PolyMesh(); //To allow derived classes to override
      
      void DoDraw(ShaderProg& shader);

    protected:
      GLuint fVAO; //Location of vertex array object from opengl. 
      GLuint fVBO; //Location of vertex buffer object from opengl.  
      GLuint fEBO; //Location of element buffer object from opengl.  

      std::vector<int> fNVertices; //Number of vertices in each polygon.  Using int instead of size_t for compatibility with opengl
      std::vector<GLuint*> fIndexOffsets; //Pointer to the beginning of the indices for each polygon

    private:
      template <class CONTAINER, class INDICES> //CONTAINER is any container with begin() and end() members
                                                //INDICES is any container with begin() and end() members 
                                                //whose elements ALSO have begin() and end() members
      void Init(const CONTAINER& vertices, const INDICES& indices, const glm::vec4& color)
      {
        std::vector<Vertex> vertexData;
        for(const auto& vertex: vertices)
        {
          Vertex vert;
          vert.position = vertex;
          vert.color = color;
          vertexData.push_back(vert);
        }

        //Construct containers of all of the indices concatenated and the number of indices in each index array
        std::vector<GLuint> flatIndices;
        GLuint indexPos = 0;
        for(const auto& indexArr: indices)
        {
          flatIndices.insert(flatIndices.end(), indexArr.begin(), indexArr.end());
          fNVertices.push_back(indexArr.size());
          indexPos += indexArr.size();
          fIndexOffsets.push_back((GLuint*)(indexPos*sizeof(GLuint)));
        }

        //Set up vertices for drawing. 
        glGenVertexArrays(1, &fVAO); 
        glBindVertexArray(fVAO);

        //Set up indices for drawing
        glGenBuffers(1, &fEBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, fEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, flatIndices.size()*sizeof(GLuint), &flatIndices[0], GL_STATIC_DRAW);

        //Construct buffer for vertices 
        glGenBuffers(1, &fVBO);
        glBindBuffer(GL_ARRAY_BUFFER, fVBO);

        glBufferData(GL_ARRAY_BUFFER, vertexData.size()*sizeof(Vertex), &vertexData[0], GL_STATIC_DRAW);

        //Set up vertex attributes expected by vertex shader
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(0));

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(sizeof(glm::vec3)));

        //glBindVertexArray(0); //TODO: This freezes the TreeView.  Is there a memory leak here?
      }
  };
}

#endif //MYGL_GEOMESH_H
