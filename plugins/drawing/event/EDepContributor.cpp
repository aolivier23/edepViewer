//File: EDepContributor.cpp
//Brief: Interface between drawing code and event display window.  A EDepContributor is given a chance to request one or more 
//       Scene names from a Viewer.  Then, the main window tells the input provider to give the EDepContributor a source of data 
//       to draw, and the EDepContributor is given a chance to remove old objects and add new objects to its Scene(s).  
//       EDepContributor is the abstract base class for all plugins that can be used with the edepsim event display. 
//Author: Andrew Olivier aolivier@ur.rochester.edu

//plugin includes
#include "EDepContributor.h"

//gl includes
#include "gl/model/Path.h"
#include "gl/model/Noop.h"

//edepsim includes
#include "TG4Event.h"

namespace draw
{
  EDepContributor::EDepContributor(const YAML::Node& config): fPDGToColor(),
                                                              fLineWidth(0.008), 
                                                              fMinLength(1.0),
                                                              fDefaultDraw(false),
                                                              fEDepRecord(new EDepRecord())
  {    
    if(config["LineWidth"]) fLineWidth = config["LineWidth"].as<float>();
    if(config["MinLength"]) fMinLength = config["MinLength"].as<float>();
    if(config["DefaultDraw"]) fDefaultDraw = config["DefaultDraw"].as<bool>();
  }

  legacy::scene_t& EDepContributor::doRequestScene(mygl::Viewer& viewer)
  {
    //Configure energy deposit Scene
    return viewer.MakeScene("EDepContributor", fEDepRecord, INSTALL_GLSL_DIR "/colorPerVertex.frag", INSTALL_GLSL_DIR "/colorPerVertex.vert", 
                            INSTALL_GLSL_DIR "/wideLine.geom");
  }

  std::unique_ptr<legacy::model_t> EDepContributor::doDraw(const TG4Event& data, Services& services)
  {
    auto model = std::make_unique<legacy::model_t>(fEDepRecord);
    auto& scene = *model;

    //Draw true energy deposits color-coded by dE/dx
    auto edepToDet = data.SegmentDetectors; //A map from sensitive volume to energy deposition

    for(auto& det: edepToDet)
    {
      auto detName = det.first;
      auto detRow = scene.emplace(fDefaultDraw);
      auto edeps = det.second;

      detRow[fEDepRecord->fPrimName] = detName;
      double sumE = 0., sumScintE = 0., minT = 1e10;

      //Produce a map of true trajectory to model_t::view
      std::map<int, legacy::model_t::view> idToIter;

      for(auto& edep: edeps)
      {
        #ifdef EDEPSIM_FORCE_PRIVATE_FIELDS
        const auto start = edep.GetStart();
        #else 
        const auto start = edep.Start;
        #endif
        glm::vec3 firstPos(start.X(), start.Y(), start.Z());
        #ifdef EDEPSIM_FORCE_PRIVATE_FIELDS
        const auto stop = edep.GetStop();
        #else
        const auto stop = edep.Stop;
        #endif

        if((stop-start).Vect().Mag() < fMinLength) continue;
          
        //Get the weighted density of the material that most of this energy deposit was deposited in.  Increase the accuracy of this 
        //material guess by increasing the number of sample points, but beware of event loading time!
        /*const size_t nSamples = 1;
        const auto diff = start.Vect()-stop.Vect();
        const double dist = diff.Mag();
        double sumDensity = 0;
        double sumA = 0;
        double sumZ = 0;   
        for(size_t sample = 0; sample < nSamples; ++sample)
        {
          std::cout << "Sample " << sample << " of material for dEdx.\n";
          const auto pos = start.Vect()+diff.Unit()*dist;
          const auto mat = gGeoManager->FindNode(pos.X(), pos.Y(), pos.Z())->GetVolume()->GetMaterial(); //TODO: Come up with a 
                                                                                                         //      solution that avoids 
                                                                                                         //      gGeoManager
          std::cout << "Found node in dE/dx sampling loop.\n";
          sumDensity += mat->GetDensity();
          sumA += mat->GetA();
          sumZ += mat->GetZ();
        }
        const double density = sumDensity/nSamples/6.24e24*1e6;*/

        #ifdef EDEPSIM_FORCE_PRIVATE_FIELDS
        const auto id = edep.GetPrimaryId();
        #else
        const auto id = edep.PrimaryId;
        #endif
        auto found = idToIter.find(id);
        if(found == idToIter.end())
        {
          found = idToIter.emplace(std::make_pair(id, detRow.emplace<mygl::Noop>(true))).first;
          auto row = found->second;
          row[fEDepRecord->fScintE] = 0;
          #ifdef EDEPSIM_FORCE_PRIVATE_FIELDS
          row[fEDepRecord->fEnergy] = data.Trajectories[id].GetInitialMomentum().E();
          row[fEDepRecord->fdEdx] = data.Trajectories[id].GetInitialMomentum().E()/
                                    (data.Trajectories[id].Points.front().GetPosition()
                                     -data.Trajectories[id].Points.back().GetPosition()).Vect().Mag();
          row[fEDepRecord->fT0] = data.Trajectories[id].Points.front().GetPosition().T();
          row[fEDepRecord->fPrimName] = data.Trajectories[id].GetName();
          #else
          row[fEDepRecord->fEnergy] = data.Trajectories[id].InitialMomentum.E();
          row[fEDepRecord->fdEdx] = data.Trajectories[id].InitialMomentum.E()/
                                    (data.Trajectories[id].Points.front().Position
                                     -data.Trajectories[id].Points.back().Position).Vect().Mag();
          row[fEDepRecord->fT0] = data.Trajectories[id].Points.front().Position.T();
          row[fEDepRecord->fPrimName] = data.Trajectories[id].Name;
          #endif
        }
        auto parent = found->second;
  
        glm::vec3 lastPos(stop.X(), stop.Y(), stop.Z());
        #ifdef EDEPSIM_FORCE_PRIVATE_FIELDS
        const double energy = edep.GetEnergyDeposit();
        const double length = edep.GetTrackLength();
        #else
        const double energy = edep.EnergyDeposit;
        const double length = edep.TrackLength;
        #endif
        //double dEdx = 0.;
        //From http://pdg.lbl.gov/2011/reviews/rpp2011-rev-passage-particles-matter.pdf, the Bethe formula for dE/dx in 
        //MeV*cm^2/g goes as Z/A.  To get comparable stopping powers for all materials, try to "remove the Z/A dependence".
        //if(length > 0.) dEdx = energy/length*10./density*sumA/sumZ;
        double dEdx = energy/length;

        #ifdef EDEPSIM_FORCE_PRIVATE_FIELDS
        const auto pdg = data.Trajectories[id].GetPDGCode();
        #else
        const auto pdg = data.Trajectories[id].PDGCode;
        #endif

        auto row = parent.emplace<mygl::Path>(true, glm::mat4(), std::vector<glm::vec3>{firstPos, lastPos}, 
                                              glm::vec4((*(services.fPDGToColor))[pdg], 1.0), 
                                              fLineWidth);
        #ifdef EDEPSIM_FORCE_PRIVATE_FIELDS
        const auto secondE = edep.GetSecondaryDeposit();
        #else
        const auto secondE = edep.SecondaryDeposit;
        #endif

        row[fEDepRecord->fScintE]  = secondE;
        row[fEDepRecord->fEnergy]  = energy;
        row[fEDepRecord->fdEdx]    = dEdx;
        row[fEDepRecord->fT0]      = start.T();
        #ifdef EDEPSIM_FORCE_PRIVATE_FIELDS
        row[fEDepRecord->fPrimName] = data.Trajectories[id].GetName(); //TODO: energy depositions children of contributing tracks?
        #else
        row[fEDepRecord->fPrimName] = data.Trajectories[id].Name;
        #endif

        sumE += energy;
        sumScintE += secondE;
        if(start.T() < minT) minT = start.T();
      }
    }
    return model;
  }

  REGISTER_EVENT(EDepContributor);
}
