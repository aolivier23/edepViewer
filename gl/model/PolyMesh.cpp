//File: PolyMesh.cpp
//Brief: Drawable set of vertices.  This interface should let me draw TGeoShapes (and therefore TGeoVolumes) with my own opengl code.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//local includes
#include "PolyMesh.h"
#include "ShaderProg.h"

//ROOT includes
#include "TGeoVolume.h"
#include "TBuffer3D.h"

//c++ includes
#include <iostream>
#include <set>

namespace mygl
{
  PolyMesh::PolyMesh(const glm::mat4& model, TGeoVolume* vol, const glm::vec4& color): Drawable(model)
  {
    if(vol == nullptr) std::cerr << "Volume is invalid!  Trouble is coming...\n"; //TODO: Throw exception
    const auto& buf = vol->GetShape()->GetBuffer3D(TBuffer3D::kRaw | TBuffer3D::kRawSizes, true);
    auto points = buf.fPnts; //Points to draw?
    auto nPts = buf.NbPnts(); //Number of points to draw?
    auto segs = buf.fSegs;
    //auto nSegs = buf.NbSegs();
    auto pols = buf.fPols;
    auto nPols = buf.NbPols();

    //Put points into a std::vector for now
    std::vector<glm::vec3> ptsVec;
    for(size_t pt = 0; pt < nPts; ++pt) ptsVec.push_back(glm::vec3(points[3*pt], points[3*pt+1], points[3*pt+2]));

    //Construct nested vector of indices.  Each vector corresponds to the indices needed by one polygon
    std::vector<std::vector<unsigned int>> indices;
    size_t polPos = 0; //Position in the array of polygon "components".  See https://root.cern.ch/doc/master/viewer3DLocal_8C_source.html
    for(size_t pol = 0; pol < nPols; ++pol)
    {
      size_t nVertices = pols[polPos+1]; //The second "component" of each polygon is the number of vertices it contains
      std::vector<unsigned int> thisPol; //Collect the unique vertices in this polygon in the order they were intended for drawing
      std::set<unsigned int> indicesFound; //Make sure that each index appears only once, but in eactly the order they appeared
      //TODO: This algorithm is STILL wrong
      for(size_t vert = 0; vert < nVertices; ++vert)
      {
        const auto seg = pols[polPos+2+vert];
        auto segToAdd = segs[1+seg*3];
        if(indicesFound.count(segToAdd) == 0)
        {
          thisPol.push_back(segToAdd);
        }
        indicesFound.insert(segToAdd);
        segToAdd = segs[2+seg*3];
        if(indicesFound.count(segToAdd) == 0)
        {
          thisPol.push_back(segToAdd);
        }
        indicesFound.insert(segToAdd);
      }
      polPos += nVertices+2;
      indices.push_back(thisPol);
    }

    //Print out ptsVec for debugging
    /*std::cout << "Printing " << ptsVec.size() << " points from volume " << vol->GetName() << ":\n";
    for(const auto& point: ptsVec) std::cout << "(" << point.x << ", " << point.y << ", " << point.z << ")\n";

    std::cout << "Printing " << nPols << " polygons from volume " << vol->GetName() << ":\n";
    for(size_t polInd = 0; polInd < indices.size(); ++polInd)
    {
      std::cout << "Polygon " << polInd << ":\n";
      for(auto index: indices[polInd])
      {
        auto point = ptsVec[index];
        std::cout << "(" << point.x << ", " << point.y << ", " << point.z << ")\n";
      }
      std::cout << "\n";
    }*/
    Init(ptsVec, indices, color);
  }

  PolyMesh::~PolyMesh()
  {
    glDeleteVertexArrays(1, &fVAO);
    glDeleteBuffers(1, &fVBO);
    glDeleteBuffers(1, &fEBO);
  }

  void PolyMesh::DoDraw(ShaderProg& shader)
  {
    shader.Use();
    shader.SetUniform("userColor", fColor.r, fColor.g, fColor.b, fColor.a);

    glBindVertexArray(fVAO);

    //TODO: Set up these indices in the constructor
    std::vector<GLuint*> indexOffsets(1, nullptr);
    GLuint indexPos = 0;
    for(auto nVert = fNVertices.begin(); nVert != --(fNVertices.end()); ++nVert)
    {
      indexPos += *nVert;
      indexOffsets.push_back((GLuint*)(indexPos*sizeof(GLuint)));
    }
   
    //std::cout << "Calling glMultiDrawElements() in mygl::PolyMesh::Draw().\n"; 
    glMultiDrawElements(GL_TRIANGLE_FAN, (GLsizei*)(&fNVertices[0]), GL_UNSIGNED_INT, (const GLvoid**)(&indexOffsets[0]), fNVertices.size());
    //Note 1: See the following tutorial for comments that somewhat explain the kRaw section of TBuffer3D:
    //        https://root.cern.ch/doc/master/viewer3DLocal_8C_source.html
    //Note 2: After much digging, it appears that ROOT draws shapes using the kRaw section of TBuffer3D 
    //        with the class TGLFaceSet: https://root.cern.ch/doc/master/TGLFaceSet_8cxx_source.html#l00312
    //Note 3: I have learned a few things about the GL_POLYGONS mode used in TGLFaceSet: 
    //        a.) Drawing polygons directly should probably be avoided in modern opengl.  
    //        b.) GL_POLYGONS only works for convex polygons anyway, so it can be replaced with GL_TRIANGLE_FAN as described in 
    //            https://www.gamedev.net/topic/268653-how-to-convert-gl_polygon-to-gl_triangles/
    //        c.) Tesselation by user (=c++) code is expensive.  Tesselation is supported in GLSL in version 4.x, but 
    //            many graphics cards (including mine at the time) do not support opengl 4.
    //Thus, I am redefining the interface of mygl::PolyMesh.  It will now use GL_TRIANGLE_FAN drawing mode.
    //Try glMultiDrawArrays() (or Elements equivalent) to draw polygons from ROOT.
    glBindVertexArray(0); //Unbind data after done drawing
  }
}
