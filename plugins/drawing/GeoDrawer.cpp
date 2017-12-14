//File: GeoDrawer.cpp
//Brief: A GeoDrawer is a Drawer that draws all of the volumes in a given ROOT TGeoManager as 3D shapes using 
//       ROOT tesselation data.  It does this by translating TGeoNodes into Drawables.
//Author: Andrew Olivier aolivier@ur.rochester.edu 

//draw includes
#include "plugins/drawing/GeoDrawer.h"

//util includes
#include "util/ColorIter.cxx"

//gl includes
#include "gl/ColRecord.cpp"
#include "gl/model/PolyMesh.h"

//ROOT includes
#include "TGeoNode.h"
#include "TGeoMatrix.h"

//gtkmm includes
#include <gtkmm.h>

//glm includes
#include <glm/gtc/type_ptr.hpp>

//c++ includes
#include <iostream>

namespace draw
{
  GeoDrawer::GeoDrawer(): fMaxDepth(7), fColor(new mygl::ColorIter())
  {
    //TODO: Configuration
  }

  void GeoDrawer::AppendChildren(mygl::Viewer& viewer, mygl::VisID& nextID, const Gtk::TreeModel::Row& parent, 
                                 TGeoNode* parentNode, TGeoMatrix& mat, size_t depth)
  {
    auto children = parentNode->GetNodes();
    if(depth == fMaxDepth) return;
    for(auto child: *children) AppendNode(viewer, nextID, (TGeoNode*)(child), mat, parent, depth+1);
  }

  Gtk::TreeModel::Row GeoDrawer::AppendNode(mygl::Viewer& viewer, mygl::VisID& nextID, TGeoNode* node, 
                                            TGeoMatrix& mat, const Gtk::TreeModel::Row& parent, size_t depth)
  {
    //Get the model matrix for node using it's parent's matrix
    TGeoHMatrix local(*(node->GetMatrix())); //Update TGeoMatrix for this node
    local.MultiplyLeft(&mat);
    double matPtr[16] = {};
    local.GetHomogenousMatrix(matPtr);

    //TODO: Just take the row to add rather than the parent as an argument to AddDrawable().
    //TODO: Adding the line immediately below this causes weird GUI behavior.  The problem does not seem to be in Scene or Polymesh::Draw().  
    auto row = viewer.AddDrawable<mygl::PolyMesh>("Geometry", nextID++, parent, false, glm::make_mat4(matPtr),
                                                  node->GetVolume(), glm::vec4((glm::vec3)(*fColor), 0.2));

    row[fGeoRecord.fName] = node->GetName();
    row[fGeoRecord.fMaterial] = node->GetVolume()->GetMaterial()->GetName();
    ++(*fColor);
    AppendChildren(viewer, nextID, row, node, local, depth);

    return row;
  }

  void GeoDrawer::doDrawEvent(const TGeoManager& data, mygl::Viewer& viewer, mygl::VisID& nextID)
  { 
    viewer.GetScenes().find("Geometry")->second.RemoveAll();

    auto id = new TGeoIdentity(); //This should be a memory leak in any reasonable framework, but TGeoIdentity registers itself
                                  //with TGeoManager so that TGeoManager will try to delete it in ~TGeoManager().  Furthermore, 
                                  //I have yet to find a way to unregister a TGeoMatrix.  So, it appears that there is no such 
                                  //thing as a temporary TGeoIdentity.  Good job ROOT... :(
    auto top = *(viewer.GetScenes().find("Geometry")->second.NewTopLevelNode());
    top[fGeoRecord.fName] = data.GetTitle();
    top[fGeoRecord.fMaterial] = "FIXME";
    AppendNode(viewer, nextID, data.GetTopNode(), *id, top, 0);
  }

  void GeoDrawer::doRequestScenes(mygl::Viewer& viewer)
  {
    auto& geoTree = viewer.MakeScene("Geometry", fGeoRecord, "/home/aolivier/app/evd/src/gl/shaders/colorPerVertex.frag", "/home/aolivier/app/evd/src/gl/shaders/colorPerVertex.vert", "/home/aolivier/app/evd/src/gl/shaders/triangleBorder.geom");
    geoTree.append_column("Volume Name", fGeoRecord.fName);
    geoTree.append_column("Material", fGeoRecord.fMaterial);
  }
}
