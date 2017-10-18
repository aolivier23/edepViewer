//File: GeoDrawer.cpp
//Brief: Tests whether the GeoMesh class can draw a volume from the DO geometry provided in ROOT examples.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//c++ includes
#include <iostream>
#include <memory>
#include <set>

//local includes
#include "gl/model/ShaderProg.h"
#include "gl/model/PolyMesh.h"

//gtkmm includes
#include <gtkmm.h>

//ROOT includes
#include "TGeoManager.h"
#include "TGeoVolume.h"
#include "TGeoShape.h"
#include "TBuffer3D.h"

namespace mygl
{
  //Implements DRAWER contract from GLCameraArea
  class GeoDrawer
  {
    public:
      GeoDrawer(): fGeoms(),
              fShader("/home/aolivier/app/opengl/gtkmm/src/ROOTGeo/shaders/camera.vert", 
                      "/home/aolivier/app/opengl/gtkmm/src/ROOTGeo/shaders/userColor.frag")
      {
        //auto man = TGeoManager::Import("/home/aolivier/ND_Studies/geo/maindet.gdml");
        //AddGeo(gGeoManager->GetVolume("KLOESolenoidCryostatOuterWall_volume")); //TODO: If I add this in the constructor, it gets drawn!  
                                                                                  //      But, I still can't see anything added after the 
                                                                                  //      constructor.
      }

      void AddGeo(TGeoVolume* vol, const glm::vec4& color)
      {
        std::cout << "Entering mygl::GeoDrawer::AddGeo()\n";
        if(vol == nullptr) std::cerr << "Volume is invalid!  Trouble is coming...\n";
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
        
        fGeoms.emplace_back(new mygl::PolyMesh(ptsVec, indices, color));
        std::cout << "Leaving mygl::GeoDrawer::AddGeo()\n";
      }

      ~GeoDrawer()
      {
        std::cout << "Destroying GeoDrawer object, so there will no longer be objects to draw for a while.\n";
      };

      void Draw(const glm::mat4& view, const glm::mat4& persp)
      {
        fShader.SetUniform("view", view);
        fShader.SetUniform("projection", persp);
        fShader.SetUniform("model", glm::mat4());

        std::cout << "Calling mygl::GeoDrawer::Draw() with " << fGeoms.size() << " objects to draw.\n";
        for(auto& geo: fGeoms) geo->Draw(fShader);
      }

    private:
      std::vector<std::unique_ptr<mygl::PolyMesh>> fGeoms; //Mesh for volume to be drawn
      mygl::ShaderProg fShader; //Shader program to be used
  };
}
