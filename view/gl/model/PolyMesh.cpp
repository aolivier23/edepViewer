//File: PolyMesh.cpp
//Brief: Drawable set of vertices.  This interface should let me draw TGeoShapes (and therefore TGeoVolumes) with my own opengl code.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//local includes
#include "PolyMesh.h"
#include "ShaderProg.h"

//c++ includes
#include <iostream>

namespace view
{
  PolyMesh::~PolyMesh()
  {
    glDeleteVertexArrays(1, &fVAO);
    glDeleteBuffers(1, &fVBO);
    glDeleteBuffers(1, &fEBO);
  }

  void PolyMesh::Draw(ShaderProg& shader)
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
   
    std::cout << "Calling glMultiDrawElements() in mygl::PolyMesh::Draw().\n"; 
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
