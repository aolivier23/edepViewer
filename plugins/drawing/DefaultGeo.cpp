//File: DefaultGeo.cpp
//Brief: A DefaultGeo is a Drawer that draws all of the volumes in a given ROOT TGeoManager as 3D shapes using 
//       ROOT tesselation data.  It does this by translating TGeoNodes into Drawables.
//Author: Andrew Olivier aolivier@ur.rochester.edu 

//draw includes
#include "plugins/drawing/DefaultGeo.h"

//plugin factory for macro
#include "plugins/Factory.cpp"

//util includes
#include "util/ColorIter.cxx"

//gl includes
#include "gl/ColRecord.cpp"
#include "gl/model/PolyMesh.h"

//ROOT includes
#include "TGeoNode.h"
#include "TGeoMatrix.h"

//glm includes
#include <glm/gtc/type_ptr.hpp>

//c++ includes
#include <iostream>

namespace draw
{
  DefaultGeo::DefaultGeo(const tinyxml2::XMLElement* config): fColor(new mygl::ColorIter()), fGeoRecord(new GeoRecord())
  {
    fMaxDepth = config->IntAttribute("MaxDepth", 7);
  }

  void DefaultGeo::AppendChildren(mygl::Scene& scene, mygl::VisID& nextID, const mygl::TreeModel::iterator parent, 
                                 TGeoNode* parentNode, TGeoMatrix& mat, size_t depth)
  {
    auto children = parentNode->GetNodes();
    if(depth == fMaxDepth) return;
    for(auto child: *children) AppendNode(scene, nextID, (TGeoNode*)(child), mat, parent, depth+1);
  }

  void DefaultGeo::AppendNode(mygl::Scene& scene, mygl::VisID& nextID, TGeoNode* node, 
                                             TGeoMatrix& mat, const mygl::TreeModel::iterator parent, size_t depth)
  {
    //Get the model matrix for node using it's parent's matrix
    TGeoHMatrix local(*(node->GetMatrix())); //Update TGeoMatrix for this node
    local.MultiplyLeft(&mat);
    double matPtr[16] = {};
    local.GetHomogenousMatrix(matPtr);

    auto iter = scene.AddDrawable<mygl::PolyMesh>(nextID++, parent, false, glm::make_mat4(matPtr),
                                                  node->GetVolume(), glm::vec4((glm::vec3)(*fColor), 0.2));
    auto& row = *iter;

    row[fGeoRecord->fName] = node->GetName();
    row[fGeoRecord->fMaterial] = node->GetVolume()->GetMaterial()->GetName();
    ++(*fColor);
    AppendChildren(scene, nextID, iter, node, local, depth);

    //return row;
  }

  void DefaultGeo::doDrawEvent(const TGeoManager& data, mygl::Viewer& viewer, mygl::VisID& nextID)
  {
    //TODO: Reset fColor here.  It probably needs to be a local variable instead of a member variable. 
    viewer.RemoveAll("Geometry");

    auto id = new TGeoIdentity(); //This should be a memory leak in any reasonable framework, but TGeoIdentity registers itself
                                  //with TGeoManager so that TGeoManager will try to delete it in ~TGeoManager().  Furthermore, 
                                  //I have yet to find a way to unregister a TGeoMatrix.  So, it appears that there is no such 
                                  //thing as a temporary TGeoIdentity.  Good job ROOT... :(
    auto& scene = viewer.GetScene("Geometry");
    auto topIter = scene.NewTopLevelNode();
    auto& top = *topIter;
    top[fGeoRecord->fName] = data.GetTitle();
    top[fGeoRecord->fMaterial] = "FIXME";
    AppendNode(scene, nextID, data.GetTopNode(), *id, topIter, 0); //TODO: Removing AppendNode() here removes unexpected GUI behavior
  }

  void DefaultGeo::doRequestScenes(mygl::Viewer& viewer)
  {
    //auto& geoTree = viewer.MakeScene("Geometry", fGeoRecord, "/home/aolivier/app/evd/src/gl/shaders/colorPerVertex.frag", "/home/aolivier/app/evd/src/gl/shaders/colorPerVertex.vert", "/home/aolivier/app/evd/src/gl/shaders/triangleBorder.geom");
    //geoTree.append_column("Volume Name", fGeoRecord.fName);
    //geoTree.append_column("Material", fGeoRecord.fMaterial);
    viewer.MakeScene("Geometry", fGeoRecord, INSTALL_GLSL_DIR "/colorPerVertex.frag", INSTALL_GLSL_DIR "/colorPerVertex.vert", 
                     INSTALL_GLSL_DIR "/triangleBorder.geom", std::unique_ptr<mygl::SceneConfig>(new GeoConfig()));
  }

  REGISTER_PLUGIN(DefaultGeo, GeoDrawer);
}

