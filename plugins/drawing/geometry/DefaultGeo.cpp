//File: DefaultGeo.cpp
//Brief: A DefaultGeo is a Drawer that draws all of the volumes in a given ROOT TGeoManager as 3D shapes using 
//       ROOT tesselation data.  It does this by translating TGeoNodes into Drawables.
//Author: Andrew Olivier aolivier@ur.rochester.edu 

//draw includes
#include "DefaultGeo.h"

//util includes
#include "util/ColorIter.cxx"

//gl includes
#include "gl/metadata/Column.cpp"
#include "gl/model/PolyMesh.h"

//ROOT includes
#include "TGeoNode.h"
#include "TGeoMatrix.h"

//glm includes
#include <glm/gtc/type_ptr.hpp>

//c++ includes
#include <iostream>

namespace
{
  glm::mat4 transposeRot(const glm::mat4& matrix)
  {
    auto result = matrix;
    for(size_t col = 0; col < 3; ++col) //3 dimensions in a rotation
    {
      for(size_t row = 0; row < 3; ++row) result[col][row] = matrix[row][col];
    }
    return result;
  }
}

namespace draw
{
  DefaultGeo::DefaultGeo(const YAML::Node& config): fMaxDepth(3), fDefaultDraw(false), fColor(new mygl::ColorIter()), fGeoRecord(new GeoRecord())
  {
    if(config["MaxDepth"]) fMaxDepth = config["MaxDepth"].as<unsigned int>();
    if(config["DefaultDraw"]) fDefaultDraw = config["DefaultDraw"].as<bool>();
  }

  void DefaultGeo::AppendChildren(legacy::model_t::view& parent, TGeoNode* parentNode, glm::mat4& mat, size_t depth)
  {
    auto children = parentNode->GetNodes();
    if(depth == fMaxDepth) return;
    for(auto child: *children) AppendNode(parent, (TGeoNode*)(child), mat, depth+1);
  }

  void DefaultGeo::AppendNode(legacy::model_t::view& parent, TGeoNode* node, glm::mat4& mat, size_t depth)
  {
    //TODO: This is actually a pretty cool way to make a basic ASCII hierarchical representation.  Maybe enable it in DEBUG mode?
    //for(size_t tab = 0; tab < depth; ++tab) std::cout << "  ";
    //std::cout << "Appending node " << node->GetName() << "\n";
    
    //Get the model matrix for node using it's parent's matrix
    double matPtr[16] = {};
    node->GetMatrix()->GetHomogenousMatrix(matPtr);
    float floatPtr[16] = {}; //TODO: There has to be a better way to do this
    for(size_t elem = 0; elem < 16; ++elem) floatPtr[elem] = matPtr[elem];

    auto local = glm::make_mat4(floatPtr);
    local = mat*::transposeRot(local);

    auto row = parent.emplace<mygl::PolyMesh>(true, local, node->GetVolume(), glm::vec4((glm::vec3)(*fColor), 0.2));

    row[fGeoRecord->fName] = node->GetName();
    row[fGeoRecord->fMaterial] = node->GetVolume()->GetMaterial()->GetName();
    ++(*fColor);
    AppendChildren(row, node, local, depth);
  }

  std::unique_ptr<legacy::model_t> DefaultGeo::doDraw(const TGeoManager& data, Services& /*services*/)
  {
    //TODO: Reset fColor here.  It probably needs to be a local variable instead of a member variable. 
    glm::mat4 id;
    auto scene = std::make_unique<legacy::model_t>(fGeoRecord);
    auto top = scene->emplace(fDefaultDraw);
    top[fGeoRecord->fName] = data.GetTitle();
    top[fGeoRecord->fMaterial] = "FIXME";
    AppendNode(top, data.GetTopNode(), id, 0); 
    return scene;
  }

  legacy::scene_t& DefaultGeo::doRequestScene(mygl::Viewer& viewer)
  {
    return viewer.MakeScene("Geometry", fGeoRecord, INSTALL_GLSL_DIR "/colorPerVertex.frag", INSTALL_GLSL_DIR "/colorPerVertex.vert", 
                            INSTALL_GLSL_DIR "/triangleBorder.geom", std::unique_ptr<mygl::SceneConfig>(new GeoConfig()));
  }

  REGISTER_GEO(DefaultGeo);
}

