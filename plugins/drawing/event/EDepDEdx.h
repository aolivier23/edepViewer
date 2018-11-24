//File: EDepDEdx.cpp
//Brief: Interface between drawing code and event display window.  A EDepDEdx is given a chance to request one or more 
//       Scene names from a Viewer.  Then, the main window tells the input provider to give the EDepDEdx a source of data 
//       to draw, and the EDepDEdx is given a chance to remove old objects and add new objects to its Scene(s).  
//       EDepDEdx is the abstract base class for all plugins that can be used with the edepsim event display. 
//Author: Andrew Olivier aolivier@ur.rochester.edu

//plugin includes
#include "EventController.cpp"

//util includes
#include "util/Palette.cpp"

#ifndef DRAW_EDEPDEDX_CPP
#define DRAW_EDEPDEDX_CPP

class TG4Event;

namespace draw
{
  class EDepDEdx
  {
    public:
      EDepDEdx(const YAML::Node& config);
      virtual ~EDepDEdx() = default;

      legacy::scene_t& doRequestScene(mygl::Viewer& viewer);
      std::unique_ptr<legacy::model_t> doDraw(const TG4Event& data, Services& services);

    private:
      //Drawing data
      mygl::Palette fPalette; //Mapping from dE/dx to color
      float fLineWidth; //The width of lines to be drawn for energy deposits
      float fMinLength; //Hit segments with length less than this value will not be drawn
      bool fDefaultDraw; //Draw this Scene by default?

      class EDepRecord: public ctrl::ColumnModel
      {
        public:
          EDepRecord(): ColumnModel(), fPrimName("Primary"), fEnergy("Energy [MeV]"), fdEdx("dE/dx"), fT0("T0 [ns]"), 
                        fScintE("Scintillation E [MeV]")
          {
            Add(fPrimName);
            Add(fEnergy);
            Add(fdEdx);
            Add(fT0);
            Add(fScintE);
            //TODO: Is it fair to call energy/length dE/dx?  
          }

          ctrl::Column<std::string> fPrimName;
          ctrl::Column<double>      fEnergy;
          ctrl::Column<double>      fdEdx;
          ctrl::Column<double>      fT0;
          ctrl::Column<double>      fScintE;
      };
      std::shared_ptr<EDepRecord> fEDepRecord;
  };
}

#endif //DRAW_EDEPDEDX_CPP 
