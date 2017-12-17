//File: EDepDEdx.cpp
//Brief: Interface between drawing code and event display window.  A EDepDEdx is given a chance to request one or more 
//       Scene names from a Viewer.  Then, the main window tells the input provider to give the EDepDEdx a source of data 
//       to draw, and the EDepDEdx is given a chance to remove old objects and add new objects to its Scene(s).  
//       EDepDEdx is the abstract base class for all plugins that can be used with the edepsim event display. 
//Author: Andrew Olivier aolivier@ur.rochester.edu

//plugin includes
#include "EventDrawer.cpp"

//util includes
#include "util/Palette.cpp"

//gl includes
#include "gl/VisID.h"
#include "gl/Viewer.h"

#ifndef DRAW_EDEPDEDX_CPP
#define DRAW_EDEPDEDX_CPP

class TGeoManager;
class TG4Event;

namespace tinyxml2
{
  class XMLElement;
}

namespace draw
{
  class EDepDEdx: public EventDrawer
  {
    public:
      EDepDEdx(const tinyxml2::XMLElement* config);
      virtual ~EDepDEdx() = default;

    protected:
      virtual void doRequestScenes(mygl::Viewer& viewer);
      virtual void doDrawEvent(const TG4Event& data, const TGeoManager& man, mygl::Viewer& viewer, 
                               mygl::VisID& nextID, Services& services);

      //Drawing data
      mygl::Palette fPalette; //Mapping from dE/dx to color
      float fLineWidth; //The width of lines to be drawn for energy deposits

      class EDepRecord: public mygl::ColRecord
      {
        public:
          EDepRecord(): ColRecord()
          {
            add(fPrimName);
            add(fEnergy);
            add(fdEdx);
            add(fT0);
            add(fScintE);
            //TODO: Is it fair to call energy/length dE/dx?  
          }

          Gtk::TreeModelColumn<double>      fEnergy;
          Gtk::TreeModelColumn<std::string> fPrimName;
          Gtk::TreeModelColumn<double>      fT0;
          Gtk::TreeModelColumn<double>      fScintE;
          Gtk::TreeModelColumn<double>      fdEdx;
      };
      EDepRecord fEDepRecord;
  };
}

#endif //DRAW_EDEPDEDX_CPP 
