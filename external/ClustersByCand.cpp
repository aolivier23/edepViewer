//File: ClustersByCand.cpp
//Brief: An ClustersByCand is an ExternalDrawer for my edepsim display that draws MCClusters from this package.  
//Author: Andrew Olivier aolivier@ur.rochester.edu

//plugin includes
#include "plugins/Factory.cpp"

//edepsim includes
#include "TG4Event.h"

//local includes
#include "external/ClustersByCand.h"

//gl includes
#include "gl/model/PolyMesh.h"

//ROOT includes
#include "TGeoBBox.h" //Easiest way to specify vertices to PolyMesh.

namespace mygl
{
  ClustersByCand::ClustersByCand(const YAML::Node& config): ExternalDrawer(config), fCands(nullptr), fClusters(nullptr),
                                                            fClusterRecord(new MCClusterRecord()), 
                                                            fCandName("CandFromTOF"), fClusterName("MergedClusters")
  {
    if(config["CandidateName"]) fCandName = config["CandidateName"].as<std::string>();

    if(config["ClusterName"]) fClusterName = config["ClusterName"].as<std::string>();
  }

  void ClustersByCand::ConnectTree(TTreeReader& reader)
  {
    std::cout << "Connecting ClustersByCand to TTreeReader.\n";
    fClusters.reset(new TTreeReaderArray<pers::MCCluster>(reader, fClusterName.c_str()));  
    fCands.reset(new TTreeReaderArray<pers::NeutronCand>(reader, fCandName.c_str()));
  }

  void ClustersByCand::doRequestScenes(mygl::Viewer& viewer)
  {
    viewer.MakeScene(fCandName, fClusterRecord, INSTALL_GLSL_DIR "/colorPerVertex.frag", INSTALL_GLSL_DIR "/colorPerVertex.vert", INSTALL_GLSL_DIR "/triangleBorder.geom");
  }

  void ClustersByCand::doDrawEvent(const TG4Event& event, mygl::Viewer& viewer, mygl::VisID& nextID, draw::Services& services)
  {
    mygl::ColorIter color; //unique color for each candidate

    auto& scene = viewer.GetScene(fCandName);
    scene.RemoveAll();

    auto topIter = scene.NewTopLevelNode();
    auto& top = *topIter;
    top[fClusterRecord->fEnergy] = std::accumulate(fCands->begin(), fCands->end(), 0., [](double value, const auto& cand) 
                                                                                       { return value + cand.DepositedEnergy; });
    top[fClusterRecord->fTime] = 0.;
    top[fClusterRecord->fParticle] = fClusterName;

    for(const auto& cand: *fCands)
    {
      auto clustAlg = cand.ClusterAlgToIndices.find(fClusterName); 
      for(const auto& elmt: clustAlg->second)
      {
        const auto& clust = (*fClusters)[elmt];
        TGeoBBox shape(clust.XWidth/2., clust.YWidth/2., clust.ZWidth/2.);
               
        glm::mat4 pos = glm::translate(glm::mat4(), glm::vec3(clust.Position.X(), clust.Position.Y(), clust.Position.Z()));

        auto& row = *(scene.AddDrawable<mygl::PolyMesh>(nextID++, topIter, fDefaultDraw, pos,
                                                        &shape, glm::vec4((glm::vec3)color, 0.3))); 

        row[fClusterRecord->fEnergy] = clust.Energy;
        row[fClusterRecord->fTime] = clust.Position.T();
        row[fClusterRecord->fParticle] = std::accumulate(clust.TrackIDs.begin(), clust.TrackIDs.end(), std::string(""),
                                                         [&event](std::string& names, const int id)
                                                         {
                                                           return names+" "+std::string(event.Trajectories[id].GetName());
                                                         });
      }
      ++color;
    }
  }

  REGISTER_PLUGIN(ClustersByCand, draw::ExternalDrawer); 
}
