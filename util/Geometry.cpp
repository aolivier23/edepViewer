//File: Geometry.cpp
//Brief: Provides access to specific geometry functions for drawing plugins.  Can cache results of expensive volume lookups 
//       since it controls the lifetime of the TGeoManager it uses.  When loading a new file, this object needs to be 
//       destroyed.  
//Author: Andrew Olivier aolivier@ur.rochester.edu

//tinyxml2 includes
#include <tinyxml2.h>

//ROOT includes
#include "TGeoManager.h"
#include "TGeoMatrix.h"
#include "TVector3.h"
#include "TGeoVolume.h"

//c++ includes
#include <memory> //For std::shared_ptr

#ifndef UTIL_GEOMETRY_CPP
#define UTIL_GEOMETRY_CPP

namespace util
{
  class Geometry
  {
    public:
      Geometry(const tinyxml2::XMLElement* config, TGeoManager* man): fFiducialNode(nullptr), fFiducialMatrix(nullptr), 
                                                                      fManager(man)
      {
        const auto fiducial = config->Attribute("fiducial");
        if(fiducial != nullptr) fFiducialName = fiducial;
        else fFiducialName = man->GetTopNode()->GetVolume()->GetName();
        SetFiducial();
      }

      virtual ~Geometry() = default;

      bool IsFiducial(const TVector3& point) 
      {
        const auto local = InLocal(point);  
        double localArr[] = {local.X(), local.Y(), local.Z()};
        return fFiducialNode->GetVolume()->Contains(localArr);        
      }

      TVector3 InLocal(const TVector3& point) 
      {
        double master[] = {point.X(), point.Y(), point.Z()};
        double local[3] = {};
        fFiducialMatrix->MasterToLocal(master, local);

        return TVector3(local[0], local[1], local[2]);
      }

      TVector3 InMaster(const TVector3& point)
      {
        double local[] = {point.X(), point.Y(), point.Z()};
        double master[3] = {};
        fFiducialMatrix->LocalToMaster(master, local);

        return TVector3(master[0], master[1], master[2]);
      }

      const TGeoNode& GetFiducial() { return *fFiducialNode; }

      const TGeoMaterial& FindMaterial(const TVector3& pos)
      {
        return *(fManager->FindNode(pos.X(), pos.Y(), pos.Z())->GetVolume()->GetMaterial());
      }

    private:
      //Fiducial node data
      const TGeoNode* fFiducialNode; //The node for the fiducial volume.  Trajectory points outside this volume will not be drawn.
      const TGeoMatrix* fFiducialMatrix; //A matrix that translates objects from the top Node's coordinate system to fFiducialNode's 
                                         //coordinate system.
      std::string fFiducialName; //The name of the fiducial volume to search for
      TGeoManager* fManager; 

      bool find_node(const TGeoNode* parent, const TGeoMatrix& mat)
      {
        TGeoHMatrix local(mat);
        std::cout << "Multiplying matrix for node " << parent->GetName() << "\n";
        local.Multiply(parent->GetMatrix());
        if(std::string(parent->GetName()) == fFiducialName)
        {
          fFiducialNode = parent;
          fFiducialMatrix = new TGeoHMatrix(local);
          return true;
        }
    
        auto children = parent->GetNodes();
        for(auto child: *children)
        {
          auto node = (TGeoNode*)child;
          if(find_node(node, local)) return true;
        }
        return false;
      }

      void SetFiducial()
      {
        const auto top = fManager->GetTopNode();
        auto id = new TGeoIdentity();
        if(!find_node(top, *id)) //TODO: Error handling?
        {
          fFiducialNode = top;
          fFiducialMatrix = new TGeoIdentity();
        }
      }
  };
}

#endif //UTIL_GEOMETRY_CPP 
