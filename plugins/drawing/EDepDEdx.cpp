//File: EDepDEdx.cpp
//Brief: Interface between drawing code and event display window.  A EDepDEdx is given a chance to request one or more 
//       Scene names from a Viewer.  Then, the main window tells the input provider to give the EDepDEdx a source of data 
//       to draw, and the EDepDEdx is given a chance to remove old objects and add new objects to its Scene(s).  
//       EDepDEdx is the abstract base class for all plugins that can be used with the edepsim event display. 
//Author: Andrew Olivier aolivier@ur.rochester.edu

//plugin includes
#include "plugins/drawing/EDepDEdx.h"
#include "plugins/Factory.cpp"

//gl includes
#include "gl/VisID.h"
#include "gl/Viewer.h"
#include "gl/model/Path.h"

//ROOT includes
#include "TGeoManager.h"

//edepsim includes
#include "TG4Event.h"

//tinyxml2 include for configuration
#include <tinyxml2.h>

namespace draw
{
  EDepDEdx::EDepDEdx(const tinyxml2::XMLElement* config): fPalette(config->FloatAttribute("dEdxMin", 0.), 
                                                                   config->FloatAttribute("dEdxMax", 8.)), 
                                                          fLineWidth(config->FloatAttribute("LineWidth", 0.008))
  {    
  }

  void EDepDEdx::doRequestScenes(mygl::Viewer& viewer)
  {
    //Configure energy deposit Scene
    auto& edepTree = viewer.MakeScene("EDep", fEDepRecord, "/home/aolivier/app/evd/src/gl/shaders/colorPerVertex.frag", "/home/aolivier/app/evd/src/gl/shaders/colorPerVertex.vert", "/home/aolivier/app/evd/src/gl/shaders/wideLine.geom");
    edepTree.append_column("Main Contributor", fEDepRecord.fPrimName);
    edepTree.append_column("Energy [MeV]", fEDepRecord.fEnergy);
    edepTree.append_column("dE/dx [MeV*cm^2/g]", fEDepRecord.fdEdx);
    edepTree.append_column("Scintillation Energy [MeV]", fEDepRecord.fScintE);
    edepTree.append_column("Start Time [ns?]", fEDepRecord.fT0);
  }

  void EDepDEdx::doDrawEvent(const TG4Event& data, const TGeoManager& man, mygl::Viewer& viewer, mygl::VisID& nextID, Services& services)
  {
    //Remove old drawing elements
    viewer.GetScenes().find("EDep")->second.RemoveAll();

    //Draw true energy deposits color-coded by dE/dx
    auto edepToDet = data.SegmentDetectors; //A map from sensitive volume to energy deposition
            
    for(auto& det: edepToDet)
    {
      auto detName = det.first;
      auto detRow = *(viewer.GetScenes().find("EDep")->second.NewTopLevelNode());
      auto edeps = det.second;

      detRow[fEDepRecord.fPrimName] = detName;
      //TODO: Map sensitive volume name to detector name and use same VisID?  This would seem to require infrastructure that 
      //      I don't yet have both in terms of Scenes communicating when an object is toggled (not too hard) and ROOT not 
      //      knowing about sensitive detector auxiliary tags (potentially very hard).  
      //TODO: Energy depositions children of sensitive detector?  This doesn't seem so useful at the moment, and sensitive detectors 
      //      do not have the "properties" I plan to include in the energy deposition tree. I will just cut out energy depositions 
      //      in volumes by setting the frustrum box from the camera API (hopefully).   
      double sumE = 0., sumScintE = 0., minT = 1e10;
                                                                                                                                                                                       
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
        const auto start = edep.Start;
        glm::vec3 firstPos(start.X(), start.Y(), start.Z());
        const auto stop = edep.Stop;
          
        //Get the density of material at the middle of this energy deposit
        /*const auto diff = start.Vect()-stop.Vect();
        const auto mid = start.Vect()+diff.Unit()*diff.Mag();
        auto mat = man.FindNode(mid.X(), mid.Y(), mid.Z())->GetVolume()->GetMaterial();
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
          const auto mat = gGeoManager->FindNode(pos.X(), pos.Y(), pos.Z())->GetVolume()->GetMaterial(); //TODO: Come up with a 
                                                                                                         //      solution that avoids 
                                                                                                         //      gGeoManager
          sumDensity += mat->GetDensity();
          sumA += mat->GetA();
          sumZ += mat->GetZ();
        }
        const double density = sumDensity/nSamples/6.24e24*1e6;
        //std::cout << "density is " << density << "\n";
  
        glm::vec3 lastPos(stop.X(), stop.Y(), stop.Z());
        const double energy = edep.EnergyDeposit;
        const double length = edep.TrackLength;
        double dEdx = 0.;
        //From http://pdg.lbl.gov/2011/reviews/rpp2011-rev-passage-particles-matter.pdf, the Bethe formula for dE/dx in 
        //MeV*cm^2/g goes as Z/A.  To get comparable stopping powers for all materials, try to "remove the Z/A dependence".
        if(length > 0.) dEdx = energy/length*10./density*sumA/sumZ; //*mat->GetA()/mat->GetZ(); 

        //TODO: Consider getting energy from total deposit, dE/dx, and primary contributor?
        /*const double eMin = 0., eMax = 1.;
        float alpha = (std::log10(dEdx)-eMin)/(eMax-eMin);
        if(alpha > 1.) alpha = 1.;
        if(alpha < 0.) alpha = 0.;*/
        
        auto row = viewer.AddDrawable<mygl::Path>("EDep", nextID, detRow, true, glm::mat4(), std::vector<glm::vec3>{firstPos, lastPos}, glm::vec4(fPalette(dEdx), 1.0), fLineWidth);
        //fPalette(dEdx), 1.0));
        //fPDGToColor[(*fCurrentEvt)->Trajectories[edep.PrimaryId].PDGCode], 1.0));
        //palette(std::log10(dEdx)), 1.0));
        //fPalette(std::log10(energy)), 1.0));
        row[fEDepRecord.fScintE]  = edep.SecondaryDeposit;
        row[fEDepRecord.fEnergy]  = energy;
        row[fEDepRecord.fdEdx]    = dEdx;
        row[fEDepRecord.fT0]      = start.T();
        row[fEDepRecord.fPrimName] = data.Trajectories[edep.PrimaryId].Name; //TODO: energy depositions children of contributing tracks?
        ++nextID;

        sumE += energy;
        sumScintE += edep.SecondaryDeposit;
        if(start.T() < minT) minT = start.T();
      }
    }
  }

  REGISTER_PLUGIN(EDepDEdx, EventDrawer);
}
