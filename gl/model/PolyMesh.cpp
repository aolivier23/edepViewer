//File: PolyMesh.cpp
//Brief: Drawable set of vertices.  This interface should let me draw TGeoShapes (and therefore TGeoVolumes) with my own opengl code.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//local includes
#include "gl/model/PolyMesh.h"
#include "gl/model/ShaderProg.h"

//ROOT includes
#include "TGeoVolume.h"
#include "TBuffer3D.h"

//c++ includes
#include <iostream>
#include <set>

//TODO: Remove me
namespace
{
  std::ostream& operator <<(std::ostream& os, const glm::vec3& vec)
  {
    os << "(" << vec.x << ", " << vec.y << ", " << vec.z << ")";
    return os;
  }
}

namespace mygl
{
  PolyMesh::PolyMesh(const glm::mat4& model, TGeoVolume* vol, const glm::vec4& color): Drawable(model), fIndexOffsets(1, nullptr)
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

    //TODO: Clockwise ordering instead of using ROOT's ordering.  From my notes below, it seems that ROOT can only 
    //      ever draw convex polygons anyway.  So, I can draw any ROOT polygon with a triangle fan provided I can wind 
    //      the vertices of that polygon in the correct order.  To determine the order of vertices, I am considering the 
    //      following algorithm:
    //      1.) Form a vector between the previous vertex and the current vertex, "prev"
    //      2.) Form a vector between the current vertex and each other vertex, "next"
    //      3.) The next vertex is the vertex such that "next" dot "prev" is > 0 and a minimum.  
    //      Is there an easier way to do this?  This sounds like a lot of vector algebra for the CPU.

    //Construct nested vector of indices.  Each vector corresponds to the indices needed by one polygon
    std::vector<std::vector<unsigned int>> indices;
    size_t polPos = 0; //Position in the array of polygon "components".  See https://root.cern.ch/doc/master/viewer3DLocal_8C_source.html
    for(size_t pol = 0; pol < nPols; ++pol)
    {
      size_t nVertices = pols[polPos+1]; //The second "component" of each polygon is the number of vertices it contains
      std::vector<unsigned int> thisPol; //Collect the unique vertices in this polygon in the order they were intended for drawing
      std::set<unsigned int> indicesFound; //Make sure that each index appears only once, but in eactly the order they appeared

      //Get the list of vertices for this polygon
      for(size_t vert = 0; vert < nVertices; ++vert)
      {
        const auto seg = pols[polPos+2+vert];
        //auto segToAdd = segs[1+seg*3];
        indicesFound.insert(segs[1+seg*3]);
        indicesFound.insert(segs[2+seg*3]);
      }
      
      //Add the vertices in counter-clockwise order
      //Find the center of the polygon as the average vertex position.
      glm::vec3 center(0., 0., 0.);
      for(const size_t index: indicesFound) center += ptsVec[index];
      center *= 1./nVertices;

      //TODO: Maybe try this: https://stackoverflow.com/questions/6989100/sort-points-in-clockwise-order
      //TODO: Algorithm gets most volumes wrong.

      //Sort vertices by angle from the first vertex
      glm::vec3 prevDir = glm::normalize(ptsVec[*(indicesFound.begin())]-center);
      std::vector<unsigned int> indicesSort(indicesFound.begin(), indicesFound.end());
      std::sort(indicesSort.begin(), indicesSort.end(), [&center, &ptsVec, &prevDir](const size_t first, const size_t second)
                                                {
                                                  const auto firstDir = glm::normalize(ptsVec[first]-center);
                                                  const auto secondDir = glm::normalize(ptsVec[second]-center);
                                                  const float firstCos = glm::dot(firstDir, prevDir);
                                                  const float secondCos = glm::dot(secondDir, prevDir);
                                                  const float firstSin = glm::length(glm::cross(prevDir, firstDir));
                                                  const float secondSin = glm::length(glm::cross(prevDir, secondDir));
                                                  std::cout << "Angle between " << firstDir << " and " << prevDir << " is " 
                                                            << atan2(firstSin, firstCos) << "\n Angle between " << secondDir 
                                                            << " and " << prevDir << " is " << atan2(secondSin, secondCos) << "\n";
                                                  return atan2(firstSin, firstCos) < atan2(secondSin, secondCos);
                                                });
      thisPol.insert(thisPol.end(), indicesSort.begin(), indicesSort.end());

      polPos += nVertices+2;
      indices.push_back(thisPol);
    }

    //Print out ptsVec for debugging
    std::cout << "Printing " << ptsVec.size() << " points from volume " << vol->GetName() << ":\n";
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
    }
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
    //shader.SetUniform("userColor", fColor.r, fColor.g, fColor.b, fColor.a);

    glBindVertexArray(fVAO);

    //TODO: GL_TRIANGLES_ADJACENCY or GL_TRIANGLE_STRIP_ADJACENCY
    glMultiDrawElements(GL_TRIANGLE_STRIP, (GLsizei*)(&fNVertices[0]), GL_UNSIGNED_INT, (const GLvoid**)(&fIndexOffsets[0]), fNVertices.size());
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
