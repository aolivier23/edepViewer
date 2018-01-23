//File: MCHitDrawer.h
//Brief: MCHitDrawer is an ExternalDrawer to work with my edepsim viewer.  It draws MCHits as Cubes using PolyMesh.  
//Author: Andrew Olivier aolivier@ur.rochester.edu

//persistency includes
#include "persistency/MCHit.h"

//draw includes
#include "plugins/drawing/ExternalDrawer.cpp"

//external evd includes
#include "util/Palette.cpp"

//tinyxml2 include for configuration
#include <tinyxml2.h>

//ROOT includes
#include "TTreeReaderArray.h"

//c++ includes
#include <memory>

#ifndef DRAW_MCHITDRAWER_H
#define DRAW_MCHITDRAWER_H

namespace mygl
{
  class ColRecord;
}

namespace mygl
{
  class MCHitDrawer: public draw::ExternalDrawer
  {
    public:
      MCHitDrawer(const tinyxml2::XMLElement* config);
      virtual ~MCHitDrawer() = default;

      virtual void ConnectTree(TTreeReader& reader) override;

      class MCHitRecord: public ColRecord
      {
        public:
          MCHitRecord(): ColRecord()
          {
            add(fEnergy);
            add(fParticle);
          }
  
          Gtk::TreeModelColumn<double> fEnergy; //The energy in this MCHit
          Gtk::TreeModelColumn<std::string> fParticle; //The most direct cause of this MCHit.  Could be the names of multiple particles
      };
      MCHitRecord fHitRecord;

    protected:
      virtual void doRequestScenes(mygl::Viewer& viewer) override;
      virtual void doDrawEvent(const TG4Event& event, mygl::Viewer& viewer, mygl::VisID& nextID, draw::Services& services) override;

    private:
      std::unique_ptr<TTreeReaderArray<pers::MCHit>> fHits;

      //Drawing data
      mygl::Palette fPalette; //Mapping from MCHit::Energy to color
  };
}

#endif //DRAW_MCHITDRAWER_H
