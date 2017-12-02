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
#include <unordered_map>

namespace
{
  //TODO: Remove me
  std::ostream& operator <<(std::ostream& os, const glm::vec3& vec)
  {
    os << "(" << vec.x << ", " << vec.y << ", " << vec.z << ")";
    return os;
  }

  struct hash
  {
    size_t operator ()(const std::pair<int, int>& pair) const
    {
      return size_t(pair.first) << 32 | pair.second; 
    }
  };
}

namespace mygl
{
  PolyMesh::PolyMesh(const glm::mat4& model, TGeoVolume* vol, const glm::vec4& color): Drawable(model), fNVertices(), fIndexOffsets()
  {
    if(vol == nullptr) std::cerr << "Volume is invalid!  Trouble is coming...\n"; //TODO: Throw exception
    const auto& buf = vol->GetShape()->GetBuffer3D(TBuffer3D::kRaw | TBuffer3D::kRawSizes, true);
    auto points = buf.fPnts; //Points to draw?
    auto nPts = buf.NbPnts(); //Number of points to draw?
    auto segs = buf.fSegs;
    //auto nSegs = buf.NbSegs();
    auto pols = buf.fPols;
    auto nPols = buf.NbPols();

    //Put points into a std::vector for now.  
    //Also find the "center" of the points.  Hopefully, this will give me a good idea of which polygons are facing "out".
    std::vector<glm::vec3> ptsVec;
    glm::vec3 ptsCenter(0.0, 0.0, 0.0);
    for(int pt = 0; pt < nPts; ++pt) 
    {
      glm::vec3 point(points[3*pt], points[3*pt+1], points[3*pt+2]);
      ptsVec.push_back(point);
      ptsCenter += point;
    }
    ptsCenter *= 1.0/ptsVec.size();

    //      Clockwise ordering instead of using ROOT's ordering.  From my notes below, it seems that ROOT can only 
    //      ever draw convex polygons anyway.  So, I can draw any ROOT polygon with a triangle fan provided I can wind 
    //      the vertices of that polygon in the correct order.  To determine the order of vertices, I am considering the 
    //      following algorithm:
    //      1.) Form a vector between the previous vertex and the current vertex, "prev"
    //      2.) Form a vector between the current vertex and each other vertex, "next"
    //      3.) The next vertex is the vertex such that "next" dot "prev" is > 0 and a minimum.  
    //      Is there an easier way to do this?  This sounds like a lot of vector algebra for the CPU.

    //Mapping from half-edge to third index of triangle
    std::unordered_map<std::pair<int, int>, int, ::hash> edgeToIndex;

    //Construct nested vector of indices.  Each vector corresponds to the indices needed by one polygon.  
    std::vector<std::vector<int>> indices;
    size_t polPos = 0; //Position in the array of polygon "components".  See https://root.cern.ch/doc/master/viewer3DLocal_8C_source.html
    for(size_t pol = 0; pol < nPols; ++pol)
    {
      size_t nVertices = pols[polPos+1]; //The second "component" of each polygon is the number of vertices it contains
      std::set<int> indicesFound; //Make sure that each index appears only once, but in eactly the order they appeared

      //Get the list of vertices for this polygon
      for(size_t vert = 0; vert < nVertices; ++vert)
      {
        const auto seg = pols[polPos+2+vert];
        indicesFound.insert(segs[1+seg*3]);
        indicesFound.insert(segs[2+seg*3]);
      }
      
      //Add the vertices in counter-clockwise order
      //Find the center of the polygon as the average vertex position.
      glm::vec3 center(0., 0., 0.);
      for(const size_t index: indicesFound) center += ptsVec[index];
      center *= 1./nVertices;
      const glm::vec3 out = glm::normalize(center-ptsCenter);
      std::cout << "The surface normal is " << out << "\n";

      //First, sort vertices by absolute angle from the first vertex
      //TODO: Apparently, this ends up giving me the ordering for GL_TRIANGLE_STRIP.  That was my ultimate goal anyway, 
      //      so sticking with it.  However, be warned that I don't REALLY know why this algorithm works the way it does.  
      //TODO: I very strongly suspect that the winding order this algorithm gives is inconsistent between edges.  This is causing 
      //      my geometry shader to get adjacency completely wrong.  
      glm::vec3 prevDir = glm::normalize(ptsVec[*(indicesFound.begin())]-center);
      std::cout << "prevDir is " << prevDir << "\n";
      std::vector<int> indicesSort(indicesFound.begin(), indicesFound.end());
      std::sort(indicesSort.begin(), indicesSort.end(), [&center, &ptsVec, &prevDir](const size_t first, const size_t second)
                                                {
                                                  const auto firstDir = glm::normalize(ptsVec[first]-center);
                                                  const auto secondDir = glm::normalize(ptsVec[second]-center);
                                                  const float firstCos = glm::dot(firstDir, prevDir);
                                                  const float secondCos = glm::dot(secondDir, prevDir);
                                                  const float firstSin = glm::length(glm::cross(firstDir, prevDir));
                                                  const float secondSin = glm::length(glm::cross(secondDir, prevDir));
                                                  const float firstAngle = atan2(firstSin, firstCos);
                                                  const float secondAngle = atan2(secondSin, secondCos);
                                                  return firstAngle < secondAngle;
                                                });

      //Next, make sure that pairs of vertices alternate sides of the polygon in the winding order 
      //TODO: I think I am getting first and last entries in adjacent polygons that match.  This method should prevent that.
      for(auto first = indicesSort.begin()+1; first < indicesSort.end()-1; first += 2)
      {
        auto second = first+1;
        const auto firstDir = glm::normalize(ptsVec[*first]-center);
        std::cout << "firstDir is " << firstDir << "\n";
        const auto secondDir = glm::normalize(ptsVec[*second]-center);
        std::cout << "secondDir is " << secondDir << "\n";
        const float projFromCenter = glm::dot(glm::cross(prevDir, firstDir), out);
        std::cout << "projFromCenter is " << projFromCenter << "\n";
        if(projFromCenter < 0) 
        {
          std::cout << "Swapping indices " << *first << " and " << *second << "\n";
          const int tmp = *first;
          *first = *second;
          *second = tmp;
          std::cout << "After the swap, projFromCenter is " << glm::dot(glm::cross(prevDir, glm::normalize(ptsVec[*first]-center)), out) << "\n";
        }
      }

      //Fill map with HalfEdges
      const auto begin = indicesSort.begin();
      const auto end = indicesSort.end()-2;

      //Special case for second edge of first triangle in each polygon
      edgeToIndex[std::make_pair(*(begin), *(begin+1))] = *(begin+2);
      std::cout << "Entered index " << *(begin+2) << " for edge (" << *(begin) << ", " << *(begin+1) << ")\n";

      for(auto index = begin; index < end; ++index)
      { 
        //Only include edges on the border
        edgeToIndex[std::make_pair(*(index+2), *index)] = *(index+1);
        std::cout << "Entered index " << *(index+1) << " for edge (" << *(index+2) << ", " << *(index) << ")\n";
      }

      //Special case for second edge of last triangle in each polygon
      edgeToIndex[std::make_pair(*(end), *(end+1))] = *(end-1);
      std::cout << "Entered index " << *(end-1) << " for edge (" << *(end) << ", " << *(end+1) << ")\n";

      polPos += nVertices+2;
      indices.push_back(indicesSort);
    }

    for(auto& pol: indices)
    {
      const auto polCopy = pol;
      std::cout << "Before I add any indices to pol, polCopy is:\n(";
      for(const auto& index: polCopy)
      {
        std::cout << index << ", ";
      }
      std::cout << ")\n";

      pol.clear();
      pol.push_back(polCopy[0]);
      if(edgeToIndex.find(std::make_pair(polCopy[1], polCopy[0])) == edgeToIndex.end()) std::cout << "Could not find index across from "
                                                                                                  << "edge (" << polCopy[0] << ", "
                                                                                                  << polCopy[1] << ")\n";
      pol.push_back(edgeToIndex[std::make_pair(polCopy[1], polCopy[0])]); 
      std::cout << "Looked up value " << edgeToIndex[std::make_pair(polCopy[1], polCopy[0])] << " for edge (" << polCopy[0] << ", "
                << polCopy[1] << ")\n";
      pol.push_back(polCopy[1]);

      if(edgeToIndex.find(std::make_pair(polCopy[0], polCopy[2])) == edgeToIndex.end()) std::cout << "Could not find index across from "
                                                                                                  << "edge (" << polCopy[2] << ", "
                                                                                                  << polCopy[0] << ")\n";
      pol.push_back(edgeToIndex[std::make_pair(polCopy[0], polCopy[2])]);
      std::cout << "Looked up value " << edgeToIndex[std::make_pair(polCopy[0], polCopy[2])] << " for edge (" << polCopy[2] << ", "
                << polCopy[0] << ")\n";
      pol.push_back(polCopy[2]);

      for(auto indexIt = polCopy.begin()+1; indexIt < polCopy.end()-2; ++indexIt)
      {
        if(edgeToIndex.find(std::make_pair(*(indexIt), *(indexIt+2))) == edgeToIndex.end()) std::cout << "Could not find index across from "
                                                                                                      << "edge (" << *(indexIt+2) << ", "
                                                                                                      << *(indexIt) << ")\n";
        pol.push_back(edgeToIndex[std::make_pair(*(indexIt), *(indexIt+2))]);
        std::cout << "Looked up value " << edgeToIndex[std::make_pair(*(indexIt), *(indexIt+2))] << " for edge (" << *(indexIt+2) << ", "
                  << *indexIt << ")\n";
        pol.push_back(*(indexIt+2));
      }

      if(edgeToIndex.find(std::make_pair(*(polCopy.end()-1), *(polCopy.end()-2))) == edgeToIndex.end()) std::cout << "Could not find index across from "
                                                                                                      << "edge (" << *(polCopy.end()-2) << ", "
                                                                                                      << *(polCopy.end()-1) << ")\n";
      pol.push_back(edgeToIndex[std::make_pair(*(polCopy.end()-1), *(polCopy.end()-2))]);
      std::cout << "Looked up value " << edgeToIndex[std::make_pair(*(polCopy.end()-1), *(polCopy.end()-2))] << " for edge (" << *(polCopy.end()-2) << ", "
                << *(polCopy.end()-1) << ")\n";
    }

    //Print out ptsVec for debugging
    std::cout << "Printing " << ptsVec.size() << " points from volume " << vol->GetName() << ":\n";
    for(const auto& point: ptsVec) std::cout << "(" << point.x << ", " << point.y << ", " << point.z << ")\n";

    std::cout << "Printing indices to draw " << indices.size() << " polygons:\n";
    for(const auto& pol: indices)
    {
      std::cout << "{";
      for(const size_t index: pol) std::cout << index << ", ";
      std::cout << "}\n";
    }

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

    //Use computer indices to send data to the GPU
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
    glBindVertexArray(fVAO);

    //std::cout << "Calling glMultiDrawElements() in mygl::PolyMesh::Draw().\n"; 
    glMultiDrawElements(GL_TRIANGLE_STRIP_ADJACENCY, (GLsizei*)(&fNVertices[0]), GL_UNSIGNED_INT, (const GLvoid**)(&fIndexOffsets[0]), fNVertices.size());
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
