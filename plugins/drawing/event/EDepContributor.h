//File: EDepContributor.cpp
//Brief: Interface between drawing code and event display window.  A EDepContributor is given a chance to request one or more 
//       Scene names from a Viewer.  Then, the main window tells the input provider to give the EDepContributor a source of data 
//       to draw, and the EDepContributor is given a chance to remove old objects and add new objects to its Scene(s).  
//       EDepContributor is the abstract base class for all plugins that can be used with the edepsim event display. 
//Author: Andrew Olivier aolivier@ur.rochester.edu

//plugin includes
#include "EventDrawer.cpp"

//util includes
#include "util/ColorIter.cxx"

//gl includes
#include "gl/VisID.h"
#include "gl/Viewer.h"

#ifndef DRAW_EDEPCONTRIBUTOR_CPP
#define DRAW_EDEPCONTRIBUTOR_CPP

class TGeoManager;
class TG4Event;

namespace mygl
{
  class TreeModel;
}

namespace draw
{
  class EDepContributor: public EventDrawer
  {
    public:
      EDepContributor(const YAML::Node& config);
      virtual ~EDepContributor() = default;

      virtual void RemoveAll(mygl::Viewer& viewer) override;

    protected:
      virtual void doRequestScenes(mygl::Viewer& viewer) override;
      virtual void doDrawEvent(const TG4Event& data, mygl::Viewer& viewer, 
                               mygl::VisID& nextID, Services& services) override;

      //Drawing data
      std::map<int, glm::vec3> fPDGToColor;
      mygl::ColorIter fPDGColor; //TODO: Merge this with fPDGToColor 
      float fLineWidth; //The width of lines to be drawn for energy deposits
      float fMinLength; //Hit segments with length less than this will not be drawn

      class EDepRecord: public mygl::ColRecord
      {
        public:
          EDepRecord(): ColRecord(), fEnergy("Energy [MeV]"), fPrimName("Primary"), fT0("T0 [ns]"), fScintE("Scintillator Energy [MeV]"), 
                        fdEdx("dE/dx")
          {
            Add(fPrimName);
            Add(fEnergy);
            Add(fdEdx);
            Add(fT0);
            Add(fScintE);
            //TODO: Is it fair to call energy/length dE/dx?  
          }

          mygl::TreeModel::Column<double>      fEnergy;
          mygl::TreeModel::Column<std::string> fPrimName;
          mygl::TreeModel::Column<double>      fT0;
          mygl::TreeModel::Column<double>      fScintE;
          mygl::TreeModel::Column<double>      fdEdx;
      };
      std::shared_ptr<EDepRecord> fEDepRecord;
  };
}

#endif //DRAW_EDEPCONTRIBUTOR_CPP 
