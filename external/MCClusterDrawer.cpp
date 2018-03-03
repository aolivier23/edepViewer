//File: MCClusterDrawer.cpp
//Brief: An MCClusterDrawer is an ExternalDrawer for my edepsim display that draws MCClusters from this package.  
//Author: Andrew Olivier aolivier@ur.rochester.edu

//plugin includes
#include "plugins/Factory.cpp"

//edepsim includes
#include "TG4Event.h"

//local includes
#include "external/MCClusterDrawer.h"

//gl includes
#include "gl/model/PolyMesh.h"

//ROOT includes
#include "TGeoBBox.h" //Easiest way to specify vertices to PolyMesh.
#include "TGeoCompositeShape.h"
#include "TGeoBoolNode.h"

namespace mygl
{
  MCClusterDrawer::MCClusterDrawer(const tinyxml2::XMLElement* config): ExternalDrawer(), fClusters(nullptr), fClusterName("MergedClusters")
  {
    const auto clustName = config->Attribute("ClusterName");
    if(clustName != nullptr) fClusterName = clustName;
  }

  void MCClusterDrawer::ConnectTree(TTreeReader& reader)
  {
    std::cout << "Connecting MCClustersDrawer to TTreeReader.\n";
    fClusters.reset(new TTreeReaderArray<pers::MCCluster>(reader, fClusterName.c_str())); 
  }

  void MCClusterDrawer::doRequestScenes(mygl::Viewer& viewer)
  {
    auto& clustTree = viewer.MakeScene(fClusterName, fClusterRecord, "/home/aolivier/app/evd/src/gl/shaders/colorPerVertex.frag", "/home/aolivier/app/evd/src/gl/shaders/colorPerVertex.vert", "/home/aolivier/app/evd/src/gl/shaders/triangleBorder.geom");
    clustTree.append_column("Energy", fClusterRecord.fEnergy);
    clustTree.append_column("Time", fClusterRecord.fTime);
    clustTree.append_column("Cause", fClusterRecord.fParticle);
  }

  void MCClusterDrawer::doDrawEvent(const TG4Event& event, mygl::Viewer& viewer, mygl::VisID& nextID, draw::Services& services)
  {
    mygl::ColorIter color; //unique color for each cluster

    viewer.RemoveAll(fClusterName);

    auto top = *(viewer.GetScenes().find(fClusterName)->second.NewTopLevelNode());
    top[fClusterRecord.fEnergy] = std::accumulate(fClusters->begin(), fClusters->end(), 0., [](double value, const auto& clust) { return value + clust.Energy; });
    top[fClusterRecord.fParticle] = fClusterName; //Algorithm name
    top[fClusterRecord.fTime] = 0.;

    for(const auto& clust: *fClusters)
    {
      /*if(clust.Hits.size() > 0)
      {
        //Combine all hits in this cluster into one composite shape
        //TODO: TGeoBoolNode does not delete the shapes I pass in its' constructor, but it does own them.  
        //      Retain a way to manually delete all of these objects myself?  
        const auto& tmpHit = (*fHits)[clust.Hits[0]];
        TGeoShape* shape = new TGeoBBox(tmpHit.Width/2., tmpHit.Width/2., tmpHit.Width/2.);

        //First two hits need to be merged with the right relative position w.r.t. the cluster.  Everything else falls into place after that.
        if(clust.Hits.size() > 1) 
        {
          const auto& scdHit = (*fHits)[clust.Hits[1]];
          auto scdBox = new TGeoBBox(scdHit.Width/2., scdHit.Width/2., scdHit.Width/2.);
          auto firstPos = tmpHit.Position-clust.Position;
          auto scdPos = scdHit.Position-clust.Position;
          shape = new TGeoCompositeShape("shape", new TGeoUnion(shape, scdBox, new TGeoTranslation(firstPos.X(), firstPos.Y(), firstPos.Z()), 
                                                                               new TGeoTranslation(scdPos.X(), scdPos.Y(), scdPos.Z())));

          for(auto hitPos = clust.Hits.begin()+1; hitPos != clust.Hits.end(); ++hitPos)
          {
            const auto& hit = (*fHits)[*hitPos];
            auto box = new TGeoBBox(hit.Width/2., hit.Width/2., hit.Width/2.);
            const auto localPos = hit.Position-clust.Position;
            shape = new TGeoCompositeShape("shape", new TGeoUnion(shape, box, gGeoIdentity, new TGeoTranslation(localPos.X(), localPos.Y(), localPos.Z())));
          }
        }*/

        //TODO: A more precise mesh that includes all of the MCHits in this cluster
        TGeoBBox shape(clust.XWidth/2., clust.YWidth/2., clust.ZWidth/2.);
             
        glm::mat4 pos = glm::translate(glm::mat4(), glm::vec3(clust.Position.X(), clust.Position.Y(), clust.Position.Z()));

        auto row = viewer.AddDrawable<mygl::PolyMesh>(fClusterName, nextID++, top, true, pos,
                                                      &shape, glm::vec4((glm::vec3)color, 0.3)); 

        ++color;
        row[fClusterRecord.fEnergy] = clust.Energy;
        row[fClusterRecord.fTime] = clust.Position.T();
        row[fClusterRecord.fParticle] = std::accumulate(clust.TrackIDs.begin(), clust.TrackIDs.end(), std::string(""),
                                                    [&event](std::string& names, const int id)
                                                    {
                                                      return names+" "+event.Trajectories[id].Name;
                                                    });
      /*}
      else std::cerr << "No hits in this cluster!\n";*/
    }
  }

  REGISTER_PLUGIN(MCClusterDrawer, draw::ExternalDrawer); 
}
