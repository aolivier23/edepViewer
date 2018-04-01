//File: Grids.cpp
//Brief: Interface between drawing code and event display window.  A Grids is given a chance to request one or more 
//       Scene names from a Viewer.  Then, the main window tells the input provider to give the Grids a source of data 
//       to draw, and the Grids is given a chance to remove old objects and add new objects to its Scene(s).  
//       Grids is the abstract base class for all plugins that can be used with the edepsim event display. 
//TODO: Make a Grids responsible for exactly one Scene?  This would mean separate loops for trajectory point 
//      and trajectory drawing with my current design. 
//Author: Andrew Olivier aolivier@ur.rochester.edu

//Header
#include "plugins/drawing/Grids.h"

//plugin factory for macro
#include "plugins/Factory.cpp"

//gl includes
#include "gl/ColRecord.cpp"
#include "gl/model/Grid.h"

//glm includes
#include <glm/gtc/type_ptr.hpp>

namespace draw
{
  Grids::Grids(const tinyxml2::XMLElement* config): fGuideRecord(new GuideRecord())
  {
    fLineWidth = config->FloatAttribute("LineWidth", 0.002);
  }

  void Grids::doRequestScenes(mygl::Viewer& viewer)
  {
    viewer.MakeScene("Grids", fGuideRecord, INSTALL_GLSL_DIR "/colorPerVertex.frag", INSTALL_GLSL_DIR "/HUD.vert", INSTALL_GLSL_DIR "/wideLine.geom");
    //guideTree.append_column("Name", fGuideRecord->fName);
    //guideTree.expand_to_path(Gtk::TreePath("0"));
  }

  void Grids::doDrawEvent(const TGeoManager& /*man*/, mygl::Viewer& viewer, mygl::VisID& nextID)
  {
    auto& scene = viewer.GetScene("Grids");
    scene.RemoveAll();

    auto rootIter = scene.NewTopLevelNode();
    auto& root = *rootIter;
    root[fGuideRecord->fName] = "Measurement Objects";

    const double gridSize = 1e5;
    //A 1m grid 
    auto& oneMrow = *(scene.AddDrawable<mygl::Grid>(nextID++, rootIter, false, glm::mat4(), gridSize, 1000., gridSize, 1000.,
                                                    glm::vec4(0.3f, 0.0f, 0.9f, 0.2f), fLineWidth));
    oneMrow[fGuideRecord->fName] = "1m Grid";

    //A 1dm grid 
    auto& oneDMrow = *(scene.AddDrawable<mygl::Grid>(nextID++, rootIter, false, glm::mat4(), gridSize, 100., gridSize, 100.,
                                                     glm::vec4(0.3f, 0.0f, 0.9f, 0.2f), fLineWidth));
    oneDMrow[fGuideRecord->fName] = "1dm Grid";

    //A 1cm grid
    auto& oneCMrow = *(scene.AddDrawable<mygl::Grid>(nextID++, rootIter, false, glm::mat4(), gridSize, 10., gridSize, 10.,
                                                     glm::vec4(0.3f, 0.0f, 0.9f, 0.2f), fLineWidth));
    oneCMrow[fGuideRecord->fName] = "1cm Grid";

    //A 1mm grid
    auto& oneMMrow = *(scene.AddDrawable<mygl::Grid>(nextID++, rootIter, false, glm::mat4(), gridSize, 1., gridSize, 1.,
                                                     glm::vec4(0.3f, 0.0f, 0.9f, 0.2f), fLineWidth));
    oneMMrow[fGuideRecord->fName] = "1mm Grid";
  }

  REGISTER_PLUGIN(Grids, GeoDrawer);
}
