//File: EDepContributor.cpp
//Brief: Interface between drawing code and event display window.  A EDepContributor is given a chance to request one or more 
//       Scene names from a Viewer.  Then, the main window tells the input provider to give the EDepContributor a source of data 
//       to draw, and the EDepContributor is given a chance to remove old objects and add new objects to its Scene(s).  
//       EDepContributor is the abstract base class for all plugins that can be used with the edepsim event display. 
//Author: Andrew Olivier aolivier@ur.rochester.edu

//plugin includes
#include "EventController.cpp"

//util includes
#include "util/ColorIter.cxx"

#ifndef DRAW_EDEPCONTRIBUTOR_CPP
#define DRAW_EDEPCONTRIBUTOR_CPP

class TG4Event;

namespace mygl
{
  class TreeModel;
}

namespace draw
{
  class EDepContributor
  {
    public:
      EDepContributor(const YAML::Node& config);
      virtual ~EDepContributor() = default;

      legacy::scene_t& doRequestScene(mygl::Viewer& viewer);
      std::unique_ptr<legacy::model_t> doDraw(const TG4Event& data, Services& services);

    private:
      //Drawing data
      std::map<int, glm::vec3> fPDGToColor;
      float fLineWidth; //The width of lines to be drawn for energy deposits
      float fMinLength; //Hit segments with length less than this will not be drawn
      bool fDefaultDraw; //Draw this Scene by default?

      class EDepRecord: public ctrl::ColumnModel
      {
        public:
          EDepRecord(): ColumnModel(), fEnergy("Energy [MeV]"), fPrimName("Primary"), fT0("T0 [ns]"), fScintE("Scintillator Energy [MeV]"), 
                        fdEdx("dE/dx")
          {
            Add(fPrimName);
            Add(fEnergy);
            Add(fdEdx);
            Add(fT0);
            Add(fScintE);
            //TODO: Is it fair to call energy/length dE/dx?  
          }

          ctrl::Column<double>      fEnergy;
          ctrl::Column<std::string> fPrimName;
          ctrl::Column<double>      fT0;
          ctrl::Column<double>      fScintE;
          ctrl::Column<double>      fdEdx;
      };
      std::shared_ptr<EDepRecord> fEDepRecord;
  };
}

#endif //DRAW_EDEPCONTRIBUTOR_CPP 
