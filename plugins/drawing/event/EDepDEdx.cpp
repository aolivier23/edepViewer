//File: EDepDEdx.cpp
//Brief: Interface between drawing code and event display window.  A EDepDEdx is given a chance to request one or more 
//       Scene names from a Viewer.  Then, the main window tells the input provider to give the EDepDEdx a source of data 
//       to draw, and the EDepDEdx is given a chance to remove old objects and add new objects to its Scene(s).  
//       EDepDEdx is the abstract base class for all plugins that can be used with the edepsim event display. 
//Author: Andrew Olivier aolivier@ur.rochester.edu

//plugin includes
#include "EDepDEdx.h"

//gl includes
#include "gl/model/Path.h"
#include "gl/model/Noop.h"

//edepsim includes
#include "TG4Event.h"

namespace draw
{
  EDepDEdx::EDepDEdx(const YAML::Node& config): fPalette(config["dEdxScale"]["min"].as<float>(), 
                                                         config["dEdxScale"]["max"].as<float>()), 
                                                fLineWidth(0.008), fMinLength(1.0), fDefaultDraw(true),
                                                fEDepRecord(new EDepRecord())
  {
    /*auto dEdxScale = std::make_pair(0.f, 8.f);
    if(config["dEdxScale"])
    {
      dEdxScale.first = config["dEdxScale"]["min"].as<float>();
      dEdxScale.second = config["dEdxScale"]["max"].as<float>();
    }
    fPalette = mygl::Palette(dEdxScale.first, dEdxScale.second);*/

    if(config["LineWidth"]) fLineWidth = config["LineWidth"].as<float>();
    if(config["MinLength"]) fMinLength = config["MinLength"].as<float>();    
    if(config["DefaultDraw"]) fDefaultDraw = config["DefaultDraw"].as<bool>();
  }
  
  legacy::scene_t& EDepDEdx::doRequestScene(mygl::Viewer& viewer)
  {
    //Configure energy deposit Scene
    return viewer.MakeScene("EDepDEdx", fEDepRecord, INSTALL_GLSL_DIR "/colorPerVertex.frag", INSTALL_GLSL_DIR "/colorPerVertex.vert", 
                            INSTALL_GLSL_DIR "/wideLine.geom");
  }

  std::unique_ptr<legacy::model_t> EDepDEdx::doDraw(const TG4Event& data, Services& services)
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
      //TODO: Map sensitive volume name to detector name and use same VisID?  This would seem to require infrastructure that 
      //      I don't yet have both in terms of Scenes communicating when an object is toggled (not too hard) and ROOT not 
      //      knowing about sensitive detector auxiliary tags (potentially very hard).  
      //TODO: Energy depositions children of sensitive detector?  This doesn't seem so useful at the moment, and sensitive detectors 
      //      do not have the "properties" I plan to include in the energy deposition tree. I will just cut out energy depositions 
      //      in volumes by setting the frustrum box from the camera API (hopefully).   
      double sumE = 0., sumScintE = 0., minT = 1e10;                                                                                                        
      //Produce a map of true trajectory to model_t::view
      std::map<int, legacy::model_t::view> idToIter;

      //Get minimum energy, maximum energy, and mean energy in this event for deciding on palette
      //TODO: Do I want a palette that is fixed per event instead?  This algorithm seems designed to 
      //      highlight structure rather than give an absolute energy scale. 
      //double mindEdx = 0., maxdEdx = 1e6; //, sumdEdx = 0.; //TODO: Check average dEdx and look at order of magnitude of its' ratio with 
                                                              //      maxdEdx to decide whether to use log scale.
      /*for(auto& edep: edeps)
      {
        const auto energy = edep.EnergyDeposit;
        const auto length = edep.TrackLength;
        double dEdx = (length>0)?energy/length:0;
        //sumdEdx += dEdx;
        if(energy < mindEdx) mindEdx = dEdx;
        else if(energy > maxdEdx) maxdEdx = dEdx;
      }
      Palette palette(std::log10(mindEdx), std::log10(maxdEdx));*/
                                                                                                                                                                                       
      for(auto& edep: edeps)
      {
        #ifdef EDEPSIM_FORCE_PRIVATE_FIELDS
        const auto start = edep.GetStart();
        const auto stop = edep.GetStop();
        #else
        const auto start = edep.Start;
        const auto stop = edep.Stop;
        #endif
        glm::vec3 firstPos(start.X(), start.Y(), start.Z());

        if((start-stop).Vect().Mag() < fMinLength) continue;
          
        //Get the density of material at the middle of this energy deposit
        /*const auto diff = start.Vect()-stop.Vect();
        const auto mid = start.Vect()+diff.Unit()*diff.Mag();
        const double density = mat->GetDensity()/6.24e24*1e6;*/

        //Get the weighted density of the material that most of this energy deposit was deposited in.  Increase the accuracy of this 
        //material guess by increasing the number of sample points, but beware of event loading time!
        const size_t nSamples = 10;
        const auto diff = start.Vect()-stop.Vect();
        const double dist = diff.Mag();
        double sumDensity = 0;
        double sumA = 0;
        double sumZ = 0;   
        for(size_t sample = 0; sample < nSamples; ++sample)
        {
          const auto pos = start.Vect()+diff.Unit()*dist;
          const auto& mat = services.fGeometry->FindMaterial(pos); 
          sumDensity += mat.GetDensity();
          sumA += mat.GetA();
          sumZ += mat.GetZ();
        }
        const double density = sumDensity/nSamples/6.24e24*1e6;
        //std::cout << "density is " << density << "\n";
  
        glm::vec3 lastPos(stop.X(), stop.Y(), stop.Z());

        #ifdef EDEPSIM_FORCE_PRIVATE_FIELDS
        const double energy = edep.GetEnergyDeposit();
        const double length = edep.GetTrackLength();
        #else
        const double energy = edep.EnergyDeposit;
        const double length = edep.TrackLength;
        #endif
        double dEdx = 0.;
        //From http://pdg.lbl.gov/2011/reviews/rpp2011-rev-passage-particles-matter.pdf, the Bethe formula for dE/dx in 
        //MeV*cm^2/g goes as Z/A.  To get comparable stopping powers for all materials, try to "remove the Z/A dependence".
        if(length > 0.) dEdx = energy/length*10./density*sumA/sumZ; //*mat->GetA()/mat->GetZ(); 

        //TODO: Consider getting energy from total deposit, dE/dx, and primary contributor?
        /*const double eMin = 0., eMax = 1.;
        float alpha = (std::log10(dEdx)-eMin)/(eMax-eMin);
        if(alpha > 1.) alpha = 1.;
        if(alpha < 0.) alpha = 0.;*/
        
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
        auto row = parent.emplace<mygl::Path>(true, glm::mat4(), std::vector<glm::vec3>{firstPos, lastPos}, 
                                              glm::vec4(fPalette(dEdx), 1.0), fLineWidth);
        //fPalette(dEdx), 1.0));
        //fPDGToColor[(*fCurrentEvt)->Trajectories[edep.PrimaryId].PDGCode], 1.0));
        //palette(std::log10(dEdx)), 1.0));
        //fPalette(std::log10(energy)), 1.0));

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

        //(*parent)[fEDepRecord->fScintE] += edep.SecondaryDeposit;

        sumE += energy;
        sumScintE += secondE;
        if(start.T() < minT) minT = start.T();
      }
    }
    return model;
  }

  REGISTER_EVENT(EDepDEdx);
}
