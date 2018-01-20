//File: Guides.cpp
//Brief: Interface between drawing code and event display window.  A Guides is given a chance to request one or more 
//       Scene names from a Viewer.  Then, the main window tells the input provider to give the Guides a source of data 
//       to draw, and the Guides is given a chance to remove old objects and add new objects to its Scene(s).  
//       Guides is the abstract base class for all plugins that can be used with the edepsim event display. 
//TODO: Make a Guides responsible for exactly one Scene?  This would mean separate loops for trajectory point 
//      and trajectory drawing with my current design. 
//Author: Andrew Olivier aolivier@ur.rochester.edu

//Header
#include "plugins/drawing/Guides.h"

//plugin factory for macro
#include "plugins/Factory.cpp"

//gl includes
#include "gl/ColRecord.cpp"
#include "gl/model/Grid.h"

//gtkmm includes
#include <gtkmm.h>

//glm includes
#include <glm/gtc/type_ptr.hpp>

namespace draw
{
  Guides::Guides(const tinyxml2::XMLElement* config)
  {
    fLineWidth = config->FloatAttribute("LineWidth", 0.002);
  }

  void Guides::doRequestScenes(mygl::Viewer& viewer)
  {
    auto& guideTree = viewer.MakeScene("Guides", fGuideRecord, "/home/aolivier/app/evd/src/gl/shaders/userColor.frag", "/home/aolivier/app/evd/src/gl/shaders/HUD.vert", "/home/aolivier/app/evd/src/gl/shaders/wideLine.geom");
    guideTree.append_column("Name", fGuideRecord.fName);
    guideTree.expand_to_path(Gtk::TreePath("0"));
  }

  void Guides::doDrawEvent(const TGeoManager& /*man*/, mygl::Viewer& viewer, mygl::VisID& nextID)
  {
    viewer.GetScenes().find("Guides")->second.RemoveAll();

    auto root = *(viewer.GetScenes().find("Guides")->second.NewTopLevelNode());
    root[fGuideRecord.fName] = "Measurement Objects";

    const double gridSize = 1e5;
    //A 1m grid 
    auto row = viewer.AddDrawable<mygl::Grid>("Guides", nextID++, root, false, glm::mat4(), gridSize, 1000., gridSize, 1000.,
                                               glm::vec4(0.3f, 0.0f, 0.9f, 0.2f), fLineWidth);
    row[fGuideRecord.fName] = "1m Grid";

    //A 1dm grid 
    row = viewer.AddDrawable<mygl::Grid>("Guides", nextID++, root, false, glm::mat4(), gridSize, 100., gridSize, 100.,
                                               glm::vec4(0.3f, 0.0f, 0.9f, 0.2f), fLineWidth);
    row[fGuideRecord.fName] = "1dm Grid";

    //A 1cm grid
    row = viewer.AddDrawable<mygl::Grid>("Guides", nextID++, root, false, glm::mat4(), gridSize, 10., gridSize, 10.,
                                               glm::vec4(0.3f, 0.0f, 0.9f, 0.2f), fLineWidth);
    row[fGuideRecord.fName] = "1cm Grid";

    //A 1mm grid
    row = viewer.AddDrawable<mygl::Grid>("Guides", nextID++, root, false, glm::mat4(), gridSize, 1., gridSize, 1.,
                                               glm::vec4(0.3f, 0.0f, 0.9f, 0.2f), fLineWidth);
    row[fGuideRecord.fName] = "1mm Grid";
  }

  REGISTER_PLUGIN(Guides, GeoDrawer);
}
