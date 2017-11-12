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
               const glm::vec4& color): Drawable(model), fVertices(vertices.begin(), vertices.end()), fColor(color)
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

      std::vector<glm::vec3> fVertices; //Vertices to be drawn.  
      std::vector<int> fNVertices; //Number of vertices in each polygon.  Using int instead of size_t for compatibility with opengl
      std::vector<GLuint> fIndexData; //Array of all indices to be used.  Really, 
                                            //this is constructed from a "2D" array, but 
                                            //I want to store it as a 1D array so that 
                                            //its elements are contiguous.  I just have 
                                            //to keep track of where each index array begins.  
                                            //This is handled by fNVertices.
      glm::vec4 fColor; //Color to render this whole object.  Passed as a uniform to fragment shader.

    private:
      template <class CONTAINER, class INDICES> //CONTAINER is any container with begin() and end() members
                                                //INDICES is any container with begin() and end() members 
                                                //whose elements ALSO have begin() and end() members
      void Init(const CONTAINER& vertices, const INDICES& indices, const glm::vec4& color)
      {
        fVertices.assign(vertices.begin(), vertices.end());
        fColor = color;
        //Construct containers of all of the indices concatenated and the number of indices in each index array
        for(const auto& indexArr: indices)
        {
          fIndexData.insert(fIndexData.end(), indexArr.begin(), indexArr.end());
          fNVertices.push_back(indexArr.size());
        }

        //Set up vertices for drawing. 
        glGenVertexArrays(1, &fVAO); //TODO: This line somehow causes the GUI to stop drawing in EvdWindow
        glBindVertexArray(fVAO);

        //Set up indices for drawing
        glGenBuffers(1, &fEBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, fEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, fIndexData.size()*sizeof(GLuint), &fIndexData[0], GL_STATIC_DRAW);

        //Construct buffer for vertices 
        glGenBuffers(1, &fVBO);
        glBindBuffer(GL_ARRAY_BUFFER, fVBO);

        glBufferData(GL_ARRAY_BUFFER, fVertices.size()*sizeof(glm::vec3), &fVertices[0], GL_STATIC_DRAW);

        //Set up vertex attributes expected by vertex shader
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (GLvoid*)(0));

        //glBindVertexArray(0); //TODO: This freezes the TreeView.  Is there a memory leak here?
      }
  };
}

#endif //MYGL_GEOMESH_H
