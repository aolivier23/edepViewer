//File: FiducialDrawer.cpp
//Brief: Interface between drawing code and event display window.  A FiducialDrawer is given a chance to request one or more 
//       Scene names from a Viewer.  Then, the main window tells the input provider to give the FiducialDrawer a source of data 
//       to draw, and the FiducialDrawer is given a chance to remove old objects and add new objects to its Scene(s).  
//       FiducialDrawer is the abstract base class for all plugins that can be used with the edepsim event display. 
//TODO: Make a FiducialDrawer responsible for exactly one Scene?  This would mean separate loops for trajectory point 
//      and trajectory drawing with my current design. 
//Author: Andrew Olivier aolivier@ur.rochester.edu

//plugin includes
#include "plugins/drawing/EventDrawer.cpp"

//gl includes
#include "gl/VisID.h"
#include "gl/Viewer.h"

//tinyxml2 includes
#include <tinyxml2.h>

//ROOT includes
#include "TGeoManager.h"
#include "TGeoMatrix.h"
#include "TVector3.h"
#include "TGeoVolume.h"

#ifndef DRAW_FIDUCIALDRAWER_CPP
#define DRAW_FIDUCIALDRAWER_CPP

class TG4Event;

namespace draw
{
  class FiducialDrawer: public EventDrawer
  {
    public:
      FiducialDrawer(const tinyxml2::XMLElement* config): fFiducialNode(nullptr), fFiducialMatrix(nullptr)
      {
        const auto fiducial = config->Attribute("fiducial");
        if(fiducial != nullptr) fFiducialName = fiducial;
        else fFiducialName = "volWorld";
      }

      virtual ~FiducialDrawer() = default;

    protected:
      //Provide a public interface, but call protected interface functions so that I can add 
      //behavior common to all FiducialDrawers here.
      virtual void doRequestScenes(mygl::Viewer& viewer) = 0;
      virtual void doDrawEvent(const TG4Event& data, const TGeoManager& man, mygl::Viewer& viewer, 
                               mygl::VisID& nextID, Services& services) = 0;

      //Helper function for derived classes
      bool IsFiducial(const TVector3& point, const TGeoManager& man) 
      {
        const auto local = InLocal(point, man);  
        double localArr[] = {local.X(), local.Y(), local.Z()};
        return fFiducialNode->GetVolume()->Contains(localArr);        
      }

      TVector3 InLocal(const TVector3& point, const TGeoManager& man) 
      {
        if(fFiducialNode == nullptr) SetFiducial(man); //TODO: I'd rather do this at initialization

        double master[] = {point.X(), point.Y(), point.Z()};
        double local[3] = {};
        fFiducialMatrix->MasterToLocal(master, local);

        return TVector3(local[0], local[1], local[2]);
      }

      TVector3 InMaster(const TVector3& point, const TGeoManager& man)
      {
        if(fFiducialNode == nullptr) SetFiducial(man); //TODO: I'd rather do this at initialization

        double local[] = {point.X(), point.Y(), point.Z()};
        double master[3] = {};
        fFiducialMatrix->LocalToMaster(master, local);

        return TVector3(master[0], master[1], master[2]);
      }

      const TGeoNode& GetFiducial() { return *fFiducialNode; }

    private:
      //Fiducial node data
      std::string fFiducialName; //TODO: Do I need to save this?  
      const TGeoNode* fFiducialNode; //The node for the fiducial volume.  Trajectory points outside this volume will not be drawn.
      const TGeoMatrix* fFiducialMatrix; //A matrix that translates objects from the top Node's coordinate system to fFiducialNode's 
                                         //coordinate system.

      bool find_node(const TGeoNode* parent, const TGeoMatrix& mat)
      {
        TGeoHMatrix local(mat);
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

      void SetFiducial(const TGeoManager& man)
      {
        const auto top = man.GetTopNode();
        TGeoIdentity id;
        if(!find_node(top, id))
        {
          fFiducialNode = top;
          fFiducialMatrix = new TGeoIdentity();
        }
      }
  };
}

#endif //DRAW_FIDUCIALDRAWER_CPP 
