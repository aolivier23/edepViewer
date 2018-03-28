//File: EDepContributor.cpp
//Brief: Interface between drawing code and event display window.  A EDepContributor is given a chance to request one or more 
//       Scene names from a Viewer.  Then, the main window tells the input provider to give the EDepContributor a source of data 
//       to draw, and the EDepContributor is given a chance to remove old objects and add new objects to its Scene(s).  
//       EDepContributor is the abstract base class for all plugins that can be used with the edepsim event display. 
//Author: Andrew Olivier aolivier@ur.rochester.edu

//plugin includes
#include "plugins/drawing/EDepContributor.h"
#include "plugins/Factory.cpp"

//gl includes
#include "gl/VisID.h"
#include "gl/Viewer.h"
#include "gl/model/Path.h"

//ROOT includes
#include "TGeoManager.h"

//edepsim includes
#include "EDepSim/TG4Event.h"

//tinyxml2 include for configuration
#include <tinyxml2.h>

namespace draw
{
  EDepContributor::EDepContributor(const tinyxml2::XMLElement* config): fPDGToColor(), fPDGColor(), 
                                                                        fLineWidth(config->FloatAttribute("LineWidth", 0.008)), 
                                                                        fEDepRecord(new EDepRecord)
  {    
  }

  void EDepContributor::doRequestScenes(mygl::Viewer& viewer)
  {
    //Configure energy deposit Scene
    viewer.MakeScene("EDep", fEDepRecord, INSTALL_GLSL_DIR "/colorPerVertex.frag", INSTALL_GLSL_DIR "/colorPerVertex.vert", 
                     INSTALL_GLSL_DIR "/wideLine.geom");
    /*edepTree.append_column("Main Contributor", fEDepRecord.fPrimName);
    edepTree.append_column("Energy [MeV]", fEDepRecord.fEnergy);
    edepTree.append_column("dE/dx [MeV*cm^2/g]", fEDepRecord.fdEdx);
    edepTree.append_column("Scintillation Energy [MeV]", fEDepRecord.fScintE);
    edepTree.append_column("Start Time [ns?]", fEDepRecord.fT0);*/
  }

  void EDepContributor::doDrawEvent(const TG4Event& data, mygl::Viewer& viewer, 
                                    mygl::VisID& nextID, Services& services)
  {
    //Remove old drawing elements
    viewer.RemoveAll("EDep");

    //Draw true energy deposits color-coded by dE/dx
    auto edepToDet = data.SegmentDetectors; //A map from sensitive volume to energy deposition

    for(auto& det: edepToDet)
    {
      auto detName = det.first;
      auto detIter = viewer.GetScenes().find("EDep")->second.NewTopLevelNode();
      auto& detRow = *detIter;
      auto edeps = det.second;

      detRow[fEDepRecord->fPrimName] = detName;
      double sumE = 0., sumScintE = 0., minT = 1e10;

      for(auto& edep: edeps)
      {
        const auto start = edep.Start;
        glm::vec3 firstPos(start.X(), start.Y(), start.Z());
        const auto stop = edep.Stop;
          
        //TODO: dE/dx plugin so I can use a simpler algorithm when neded.
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
  
        glm::vec3 lastPos(stop.X(), stop.Y(), stop.Z());
        const double energy = edep.EnergyDeposit;
        const double length = edep.TrackLength;
        //double dEdx = 0.;
        //From http://pdg.lbl.gov/2011/reviews/rpp2011-rev-passage-particles-matter.pdf, the Bethe formula for dE/dx in 
        //MeV*cm^2/g goes as Z/A.  To get comparable stopping powers for all materials, try to "remove the Z/A dependence".
        //if(length > 0.) dEdx = energy/length*10./density*sumA/sumZ;
        double dEdx = edep.EnergyDeposit/edep.TrackLength;

        auto iter = viewer.AddDrawable<mygl::Path>("EDep", nextID, detIter, true, glm::mat4(), std::vector<glm::vec3>{firstPos, lastPos}, glm::vec4((*(services.fPDGToColor))[data.Trajectories[edep.PrimaryId].PDGCode], 1.0), fLineWidth);
        auto& row = *iter;

        row[fEDepRecord->fScintE]  = edep.SecondaryDeposit;
        row[fEDepRecord->fEnergy]  = energy;
        row[fEDepRecord->fdEdx]    = dEdx;
        row[fEDepRecord->fT0]      = start.T();
        row[fEDepRecord->fPrimName] = data.Trajectories[edep.PrimaryId].Name; //TODO: energy depositions children of contributing tracks?
        ++nextID;

        sumE += energy;
        sumScintE += edep.SecondaryDeposit;
        if(start.T() < minT) minT = start.T();
      }
    }
  }

  REGISTER_PLUGIN(EDepContributor, EventDrawer);
}
