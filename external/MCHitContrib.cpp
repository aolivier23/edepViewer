//File: MCHitContrib.cpp
//Brief: An MCHitContrib is an ExternalDrawer for my edepsim display that draws MCHits from this package.  
//Author: Andrew Olivier aolivier@ur.rochester.edu

//The core ImGUI function definitions
#include "imgui.h"

//glm includes
#include <glm/gtc/type_ptr.hpp>

//plugin includes
#include "plugins/Factory.cpp"

//edepsim includes
#include "TG4Event.h"

//local includes
#include "external/MCHitContrib.h"

//gl includes
#include "gl/model/PolyMesh.h"

//ROOT includes
#include "TGeoBBox.h" //Easiest way to specify vertices to PolyMesh.
#include "TGeoSphere.h" //For distance particle could have travelled

//TODO: If I depend on EdepNeutrons (external package) anyway, can't I just use its' headers here?
namespace
{
  //Returns the FS TG4Trajectory that led to child.
  const TG4Trajectory& Matriarch(const TG4Trajectory& child, const std::vector<TG4Trajectory>& trajs)
  {
    #ifdef EDEPSIM_FORCE_PRIVATE_FIELDS
    if(child.GetParentId() == -1) return child;
    return Matriarch(trajs[child.GetParentId()], trajs);
    #else
    if(child.ParentId == -1) return child;
    return Matriarch(trajs[child.ParentId], trajs);
    #endif
  }
}

namespace mygl
{
  MCHitContrib::MCHitContrib(const YAML::Node& config): ExternalDrawer(config), fHits(nullptr), fHitName("NeutronHits"), 
                                                        fHitRecord(new MCHitRecord()), fContribToColor()                                                 
  {
    if(config["HitName"]) fHitName = config["HitName"].as<std::string>();
  }

  void MCHitContrib::ConnectTree(TTreeReader& reader)
  {
    std::cout << "Connecting MCHitsDrawer to TTreeReader.\n";
    fHits.reset(new TTreeReaderArray<pers::MCHit>(reader, fHitName.c_str())); //TODO: Get name of hits to process from XML file in constructor
  }

  void MCHitContrib::doRequestScenes(mygl::Viewer& viewer)
  {
    viewer.MakeScene(fHitName, fHitRecord, INSTALL_GLSL_DIR "/colorPerVertex.frag", INSTALL_GLSL_DIR "/colorPerVertex.vert", INSTALL_GLSL_DIR "/triangleBorder.geom");
    /*hitTree.append_column("Energy", fHitRecord->fEnergy);
    hitTree.append_column("Time", fHitRecord->fTime);
    hitTree.append_column("Cause", fHitRecord->fParticle);*/
  }

  void MCHitContrib::doDrawEvent(const TG4Event& event, mygl::Viewer& viewer, mygl::VisID& nextID, draw::Services& services)
  {
    mygl::ColorIter color;

    auto& scene = viewer.GetScene(fHitName);
    scene.RemoveAll();
    fContribToColor.clear();

    auto topIter = scene.NewTopLevelNode();
    auto& top = *topIter;
    top[fHitRecord->fEnergy] = std::accumulate(fHits->begin(), fHits->end(), 0., [](double value, const auto& hit) { return value + hit.Energy; });
    top[fHitRecord->fParticle] = fHitName; //Algorithm name
    top[fHitRecord->fTime] = 0.;

    const auto& trajs = event.Trajectories;

    for(const auto& hit: *fHits)
    {
      //Find the FS particle that produced this MCHit.  If there is more than one FS particle, complain via cerr.
      const auto& FSPart = ::Matriarch(trajs[hit.TrackIDs.front()], trajs);
      for(const auto& id: hit.TrackIDs)
      {
        #ifdef EDEPSIM_FORCE_PRIVATE_FIELDS
        if(::Matriarch(trajs[id], trajs).GetTrackId() != FSPart.GetTrackId()) std::cerr << "Found an MCHit with multiple FS particles!\n";
        #else
        if(::Matriarch(trajs[id], trajs).TrackId != FSPart.TrackId) std::cerr << "Found an MCHit with multiple FS particles!\n";
        #endif
      }
      FSInfo info;
      #ifdef EDEPSIM_FORCE_PRIVATE_FIELDS
      info.TrackID = FSPart.GetTrackId();
      info.Name = FSPart.GetName();
      info.Energy = FSPart.GetInitialMomentum().E() - FSPart.GetInitialMomentum().Mag();
      #else
      info.TrackID = FSPart.TrackId;
      info.Name = FSPart.Name;
      info.Energy = FSPart.InitialMomentum.E() - FSPart.InitialMomentum.Mag();
      #endif

      const auto found = fContribToColor.emplace(info, (glm::vec3)color);
      if(found.second) ++color;

      TGeoBBox box(hit.Width/2., hit.Width/2., hit.Width/2.);
           
      glm::mat4 pos = glm::translate(glm::mat4(), glm::vec3(hit.Position.X(), hit.Position.Y(), hit.Position.Z()));

      auto iter = scene.AddDrawable<mygl::PolyMesh>(nextID++, topIter, fDefaultDraw, pos,
                                                    &box, glm::vec4(found.first->second, 1.0)); 
      auto& row = *iter;
      row[fHitRecord->fEnergy] = hit.Energy;
      row[fHitRecord->fTime] = hit.Position.T();
      #ifdef EDEPSIM_FORCE_PRIVATE_FIELDS
      row[fHitRecord->fDist] = (hit.Position - FSPart.Points.front().GetPosition()).Vect().Mag();
      #else
      row[fHitRecord->fDist] = (hit.Position - FSPart.Points.front().Position).Vect().Mag();
      #endif
      row[fHitRecord->fParticle] = std::accumulate(hit.TrackIDs.begin(), hit.TrackIDs.end(), std::string(""), 
                                                  [&trajs](std::string& names, const int id)
                                                  {
                                                    #ifdef EDEPSIM_FORCE_PRIVATE_FIELDS
                                                    return names+" "+std::string(trajs[id].GetName());
                                                    #else
                                                    return names+" "+std::string(trajs[id].Name);
                                                    #endif
                                                  });

      //Produce Spheres for distance this neutron could travel in time resolution
      const double c = 30.; //speed of light in cm/ns
      #ifdef EDEPSIM_FORCE_PRIVATE_FIELDS
      const auto diff = (hit.Position - FSPart.Points.front().GetPosition());
      #else 
      const auto diff = (hit.Position - FSPart.Points.front().Position);
      #endif
      const double beta = diff.Vect().Mag()/c/diff.T();
      const double timeRes = 0.7; //Measured in test beam in ns

      if(beta > 0) 
      {
        for(size_t sigmas = 1; sigmas < 6; ++sigmas)
        {
          TGeoSphere sphere(0., beta*c*timeRes*sigmas);
          auto& row = *(scene.AddDrawable<mygl::PolyMesh>(nextID++, iter, false, pos, &sphere, glm::vec4(found.first->second, 0.2)));

          row[fHitRecord->fEnergy] = hit.Energy;
          row[fHitRecord->fTime]   = timeRes*sigmas;
          row[fHitRecord->fDist]   = beta*c*timeRes*sigmas;
          row[fHitRecord->fParticle] = std::to_string(sigmas)+" Sigmas";
        }
      }
    }
  }

  void MCHitContrib::Render()
  {
    ImGui::Begin("MCHit Contributors");
    ImGui::Columns(3);

    //Column labels
    ImGui::Text("Color");
    ImGui::NextColumn();
    
    ImGui::Text("Particle");
    ImGui::NextColumn();

    ImGui::Text("KE [MeV]");
    ImGui::NextColumn();
    ImGui::Separator();

    for(auto& pair: fContribToColor)
    {
      ImGui::ColorEdit3(std::to_string(pair.first.TrackID).c_str(), glm::value_ptr(pair.second), ImGuiColorEditFlags_NoInputs);
      ImGui::NextColumn();

      ImGui::Text(pair.first.Name.c_str());
      ImGui::NextColumn();
      
      ImGui::Text(std::to_string(pair.first.Energy).c_str());
      ImGui::NextColumn(); 
    }
    ImGui::Columns(1);
    ImGui::End();
  }

  REGISTER_PLUGIN(MCHitContrib, draw::ExternalDrawer)
}
