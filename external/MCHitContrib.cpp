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
    if(child.ParentId == -1) return child;
    return Matriarch(trajs[child.ParentId], trajs);
  }
}

namespace mygl
{
  MCHitContrib::MCHitContrib(const tinyxml2::XMLElement* config): ExternalDrawer(), fHits(nullptr), fHitName("NeutronHits"), 
                                                                  fHitRecord(new MCHitRecord()), fContribToColor()                                                 
  {
    const auto hitName = config->Attribute("HitName");
    if(hitName != nullptr) fHitName = hitName;
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
        if(::Matriarch(trajs[id], trajs).TrackId != FSPart.TrackId) std::cerr << "Found an MCHit with multiple FS particles!\n";
      }
      FSInfo info;
      info.TrackID = FSPart.TrackId;
      info.Name = FSPart.Name;
      info.Energy = FSPart.InitialMomentum.E() - FSPart.InitialMomentum.Mag();

      const auto found = fContribToColor.emplace(info, (glm::vec3)color);
      if(found.second) ++color;

      TGeoBBox box(hit.Width/2., hit.Width/2., hit.Width/2.);
           
      glm::mat4 pos = glm::translate(glm::mat4(), glm::vec3(hit.Position.X(), hit.Position.Y(), hit.Position.Z()));

      auto iter = scene.AddDrawable<mygl::PolyMesh>(nextID++, topIter, true, pos,
                                                    &box, glm::vec4(found.first->second, 1.0)); 
      auto& row = *iter;
      row[fHitRecord->fEnergy] = hit.Energy;
      row[fHitRecord->fTime] = hit.Position.T();
      row[fHitRecord->fDist] = (hit.Position - FSPart.Points.front().Position).Vect().Mag();
      row[fHitRecord->fParticle] = std::accumulate(hit.TrackIDs.begin(), hit.TrackIDs.end(), std::string(""), 
                                                  [&trajs](std::string& names, const int id)
                                                  {
                                                    return names+" "+trajs[id].Name;
                                                  });

      //Produce Spheres for distance this neutron could travel in time resolution
      const double c = 30.; //speed of light in cm/ns
      const auto diff = (hit.Position - FSPart.Points.front().Position);
      const double beta = diff.Vect().Mag()/c/diff.T();
      const double timeRes = 0.7; //Measured in test beam in ns

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

  REGISTER_PLUGIN(MCHitContrib, draw::ExternalDrawer); 
}
