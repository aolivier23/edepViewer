//File: PolyMesh.h
//Brief: A PolyMesh is a collection of vertices, with an overall color, to be drawn using opengl.  The vertices in a 
//       PolyMesh are used to create polygons that form the face of the mesh.  PolyMesh is designed to 
//       draw individual TGeoShapes from CERN's ROOT framework.  
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef MYGL_POLYMESH_H
#define MYGL_POLYMESH_H

//model includes
#include "gl/model/VAO.h"

//c++ includes
#include <vector>

//glm includes
#include <glm/glm.hpp>

//gl includes
#include "gl/model/Drawable.h"
#include "gl/objects/ShaderProg.h"

class TGeoVolume;
class TGeoShape;

namespace mygl
{
  class PolyMesh: public Drawable
  {
    public:
      //Note: I have to implement template members in the class header
      template <class CONTAINER, class INDICES> //CONTAINER is any container with begin() and end() members
                                                //INDICES is any container with begin() and end() members 
                                                //whose elements ALSO have begin() and end() members
      PolyMesh(VAO& vao, const glm::mat4& model, const CONTAINER& vertices, const INDICES& indices, 
               const glm::vec4& color): Drawable(model), fNVertices(), fIndexOffsets(1, nullptr)
      {
        Init(vao, vertices, indices, color);
      }

      PolyMesh(VAO& vao, const glm::mat4& model, TGeoVolume* vol, const glm::vec4& color);
      PolyMesh(VAO& vao, const glm::mat4& model, TGeoShape* shape, const glm::vec4& color);

      virtual ~PolyMesh(); //To allow derived classes to override
      
      void DoDraw(ShaderProg& shader);

    protected:
      std::vector<int> fNVertices; //Number of vertices in each polygon.  Using int instead of size_t for compatibility with opengl
      std::vector<GLuint*> fIndexOffsets; //Pointer to the beginning of the indices for each polygon

    private:
      template <class CONTAINER, class INDICES> //CONTAINER is any container with begin() and end() members
                                                //INDICES is any container with begin() and end() members 
                                                //whose elements ALSO have begin() and end() members
      void Init(VAO& vao, const CONTAINER& vertices, const INDICES& indices, const glm::vec4& color)
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

        fOffset = vao.Register(vertexData, flatIndices); 
        for(auto& offset: fIndexOffsets) offset += fOffset;
      }
  };
}

#endif //MYGL_GEOMESH_H
