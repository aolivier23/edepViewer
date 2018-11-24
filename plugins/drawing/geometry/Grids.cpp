//File: Grids.cpp
//Brief: Interface between drawing code and event display window.  A Grids is given a chance to request one or more 
//       Scene names from a Viewer.  Then, the main window tells the input provider to give the Grids a source of data 
//       to draw, and the Grids is given a chance to remove old objects and add new objects to its Scene(s).  
//       Grids is the abstract base class for all plugins that can be used with the edepsim event display. 
//TODO: Make a Grids responsible for exactly one Scene?  This would mean separate loops for trajectory point 
//      and trajectory drawing with my current design. 
//Author: Andrew Olivier aolivier@ur.rochester.edu

//Header
#include "Grids.h"

//gl includes
#include "gl/metadata/Column.cpp"
#include "gl/model/Grid.h"

//glm includes
#include <glm/gtc/type_ptr.hpp>

namespace draw
{
  Grids::Grids(const YAML::Node& config): fGuideRecord(new GuideRecord()), fLineWidth(0.006), fDefaultDraw(false)
  {
    if(config["LineWidth"]) fLineWidth = config["LineWidth"].as<float>();

    if(config["DefaultDraw"]) fDefaultDraw = config["DefaultDraw"].as<bool>();
  }

  legacy::scene_t& Grids::doRequestScene(mygl::Viewer& viewer)
  {
    constexpr auto name = "Grids";
    return viewer.MakeScene(name, fGuideRecord, INSTALL_GLSL_DIR "/colorPerVertex.frag", INSTALL_GLSL_DIR "/HUD.vert", 
                            INSTALL_GLSL_DIR "/wideLine.geom");
  }

  std::unique_ptr<legacy::model_t> Grids::doDraw(const TGeoManager& /*man*/, Services& /*services*/)
  {
    auto scene = std::make_unique<legacy::model_t>(fGuideRecord);
    auto root = scene->emplace(fDefaultDraw);
    root[fGuideRecord->fName] = "Measurement Objects";

    const double gridSize = 1e5;
    //A 1m grid 
    auto oneMrow = root.emplace<mygl::Grid>(false, glm::mat4(), gridSize, 1000., gridSize, 1000.,
                                             glm::vec4(0.3f, 0.0f, 0.9f, 0.2f), fLineWidth);
    oneMrow[fGuideRecord->fName] = "1m Grid";

    //A 1dm grid 
    auto oneDMrow = root.emplace<mygl::Grid>(false, glm::mat4(), gridSize, 100., gridSize, 100.,
                                                  glm::vec4(0.3f, 0.0f, 0.9f, 0.2f), fLineWidth);
    oneDMrow[fGuideRecord->fName] = "1dm Grid";

    //A 1cm grid
    auto oneCMrow = root.emplace<mygl::Grid>(false, glm::mat4(), gridSize, 10., gridSize, 10.,
                                                  glm::vec4(0.3f, 0.0f, 0.9f, 0.2f), fLineWidth);
    oneCMrow[fGuideRecord->fName] = "1cm Grid";

    //A 1mm grid
    auto oneMMrow = root.emplace<mygl::Grid>(false, glm::mat4(), gridSize, 1., gridSize, 1.,
                                                  glm::vec4(0.3f, 0.0f, 0.9f, 0.2f), fLineWidth);
    oneMMrow[fGuideRecord->fName] = "1mm Grid";

    return scene;
  }

  REGISTER_GEO(Grids);
}
