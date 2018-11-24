//File: Grids.cpp
//Brief: Interface between drawing code and event display window.  A Grids is given a chance to request one or more 
//       Scene names from a Viewer.  Then, the main window tells the input provider to give the Grids a source of data 
//       to draw, and the Grids is given a chance to remove old objects and add new objects to its Scene(s).  
//       Grids is the abstract base class for all plugins that can be used with the edepsim event display. 
//Author: Andrew Olivier aolivier@ur.rochester.edu

//Base class
#include "GeoController.cpp"

#ifndef DRAW_GRIDS_H
#define DRAW_GRIDS_H

class TGeoManager;

namespace draw
{
  class Grids
  {
    public:
      Grids(const YAML::Node& config); //Configure a new Grids
      virtual ~Grids() = default;

      //Provide a public interface, but call protected interface functions so that I can add 
      //behavior common to all Grids here.
      virtual legacy::scene_t& doRequestScene(mygl::Viewer& viewer);
      virtual std::unique_ptr<legacy::model_t> doDraw(const TGeoManager& man, Services& /*services*/);

    private:
      class GuideRecord: public ctrl::ColumnModel
      {
        public:
          GuideRecord(): ColumnModel(), fName("Name")
          {
            Add(fName);
          }

          ctrl::Column<std::string> fName;
      };

      std::shared_ptr<GuideRecord> fGuideRecord;

      //Drawing configuration
      float fLineWidth; //Width of grid lines
      bool fDefaultDraw; //Draw all objects by default
  };
}

#endif //DRAW_GRIDS_H
