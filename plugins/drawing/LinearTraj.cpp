//File: LinearTraj.cpp
//Brief: A plugin that draws true trajectories of particles.  
//Author: Andrew Olivier aolivier@ur.rochester.edu

//draw includes
#include "plugins/drawing/LinearTraj.h"

//gl includes
#include "gl/model/Path.h"
#include "gl/model/Point.h"

//plugin factory for macro
#include "plugins/Factory.cpp"

//ROOT includes
#include "TDatabasePDG.h"

//edepsim includes
#include "EDepSim/TG4Event.h" 

//tinyxml2 include for configuration
#include <tinyxml2.h>

//c++ includes
#include <regex>

namespace draw
{
  LinearTraj::LinearTraj(const tinyxml2::XMLElement* config): fTrajRecord(new TrajRecord())
  {
    fLineWidth = config->FloatAttribute("LineWidth", 0.008);
  }

  void LinearTraj::doRequestScenes(mygl::Viewer& viewer) 
  {
    //Configure trajectory Scene
    viewer.MakeScene("Trajectories", fTrajRecord, INSTALL_GLSL_DIR "/colorPerVertex.frag", INSTALL_GLSL_DIR "/colorPerVertex.vert", INSTALL_GLSL_DIR "/wideLine.geom");
    //trajTree.append_column("Particle Type", fTrajRecord.fPartName);
    //trajTree.insert_column_with_data_func(-1, "Particle", fPartNameRender, sigc::mem_fun(*this, &EvdWindow::ColToColor));
    //trajTree.append_column("KE [MeV]", fTrajRecord.fEnergy);
  }

  void LinearTraj::doDrawEvent(const TG4Event& evt, mygl::Viewer& viewer, mygl::VisID& nextID, Services& services) 
  {
    //First, clear the scenes I plan to draw on
    auto& trajScene = viewer.GetScene("Trajectories");
    trajScene.RemoveAll();

    //Next, make maps of trackID to particle and parent ID to particle
    std::map<int, std::vector<TG4Trajectory>> parentID;
    for(auto& traj: evt.Trajectories)
    {
      parentID[traj.ParentId].push_back(traj);
    }

    //Then, add particles to list tree and viewer
    std::regex genieToEvd(R"(nu:(-?[[:digit:]]+);tgt:([[:digit:]]+);N:([[:digit:]]+)(.*);proc:Weak\[([[:alpha:]]+)\],([[:alpha:]]+);(.*))");
    auto& primaries = evt.Primaries;
    for(auto& prim: primaries)
    {
      auto iter = trajScene.NewTopLevelNode();
      auto& row = *iter;
        
      //Turn GENIE's interaction string into something easier to read
      std::smatch match;
      std::string nu;
      int pdg;
      if(std::regex_match(prim.Reaction, match, genieToEvd)) 
      {

        const std::string nu = TDatabasePDG::Instance()->GetParticle(std::stoi(match[1].str()))->GetName();
        //const std::string nucleus = fPdgDB.GetParticle(match[2].str().c_str())->GetName(); //TODO: TDatabasPDG can't read PDG codes for nuclei

        /*const auto particle = TDatabasePDG::Instance()->GetParticle(std::stoi(match[3].str()));
        if(particle)
        {
          const std::string nucleon = particle->GetName(); 
        }*/

        //Add this interaction vertex to the scene of trajectory points
        pdg = std::stoi(match[1].str());
      }
      else
      {
        std::cerr << "WARNING: Got interaction string from GENIE that does not match what I expect:\n"
                  << prim.Reaction << "\n";
        nu = prim.Reaction;
        pdg = 0; //Supposed to be a "Geantino" to indicate that something is wrong
      }

      row[fTrajRecord->fPartName] = nu+" "+match[5].str()+" "+match[6].str();//+" on "/*+nucleus+" "*/+nucleon;
      row[fTrajRecord->fEnergy] = -1; //TODO: Get this from updated TG4PrimaryVertex?

      const auto ptPos = prim.Position;
      const auto color = (*(services.fPDGToColor))[pdg];

      const auto& children = parentID[-1];
      for(const auto& child: children) AppendTrajectory(viewer, nextID, iter, child, parentID, services);
    }
  }

  //Helper functions for drawing trajectories and trajectory points
  void LinearTraj::AppendTrajectory(mygl::Viewer& viewer, mygl::VisID& nextID, const mygl::TreeModel::iterator parent, 
                                    const TG4Trajectory& traj, std::map<int, std::vector<TG4Trajectory>>& parentToTraj, 
                                    Services& services)
  {
    const int pdg = traj.PDGCode;
    const auto color = (*(services.fPDGToColor))[pdg];

    auto points = traj.Points;
    std::vector<glm::vec3> vertices;
    for(auto pointIt = points.begin(); pointIt != points.end(); ++pointIt)
    {
      auto& point = *pointIt;
      const auto& pos = point.Position;

      //Require that point is inside fiducial volume
      if(services.fGeometry->IsFiducial(pos.Vect()))
      {
        vertices.emplace_back(pos.X(), pos.Y(), pos.Z());
      }
      else if(pointIt != points.begin()) //Extrapolate to the face of the fiducial volume
      {
        const auto prevPoint = (pointIt-1)->Position;

        //Make sure previous point was in the fiducial volume
        if(services.fGeometry->IsFiducial(prevPoint.Vect()))
        {
          //TGeoShape::DistFromInside needs two 3-vectors in the local frame:
          //The direction
          const auto dirLocalV = services.fGeometry->InLocal((pos-prevPoint).Vect().Unit());
          double dirLocal[] = {dirLocalV.X(), dirLocalV.Y(), dirLocalV.Z()};

          //The position inside the volume
          const auto prevLocal = services.fGeometry->InLocal(prevPoint.Vect());          
          double local[] = {prevLocal.X(), prevLocal.Y(), prevLocal.Z()};

          const auto dist = services.fGeometry->GetFiducial().GetVolume()->GetShape()->DistFromInside(local, dirLocal, 3); 

          //Now, convert the position of the point to draw back to the master frame in which everything is drawn          
          const auto master = services.fGeometry->InMaster(TVector3(local[0]+dirLocal[0]*dist, local[1]+dirLocal[1]*dist, 
                                                                    local[2]+dirLocal[2]*dist));
          vertices.emplace_back(master.X(), master.Y(), master.Z());
        }
      }
    }

    auto iter = viewer.GetScene("Trajectories").AddDrawable<mygl::Path>(nextID, parent, true, glm::mat4(), vertices, 
                                                                        glm::vec4((glm::vec3)color, 1.0), fLineWidth); 
    auto& row = *iter;
    row[fTrajRecord->fPartName] = traj.Name;
    auto p = traj.InitialMomentum;
    const double invariantMass = std::sqrt((p.Mag2()>0)?p.Mag2():0); //Make sure the invariant mass is > 0 as it should be.  It might be negative for 
                                                                       //photons because of floating point precision behavior.  Never trust a computer to 
                                                                       //calculate 0...
    row[fTrajRecord->fEnergy] = p.E()-invariantMass; //Kinetic energy
    ++nextID;

    //Second pass over trajectory points to find starting points of children.
    auto children = parentToTraj[traj.TrackId];

    //Produce map of closest trajectory point to child trajectory
    std::map<std::vector<TG4TrajectoryPoint>::iterator, std::vector<TG4Trajectory>> ptToTraj;
    for(const auto& child: children)
    {
      ptToTraj[std::min_element(points.begin(), points.end(), [&child](const TG4TrajectoryPoint& first, 
                                                                       const TG4TrajectoryPoint& second)
                                                              {
                                                                return (first.Position - child.Points.front().Position).Mag() < 
                                                                       (second.Position - child.Points.front().Position).Mag();
                                                              })].push_back(child);
    } 
      
    for(auto ptIt = points.begin(); ptIt != points.end(); ++ptIt)
    {
      const auto& point = *ptIt;
      const auto& subChildren = ptToTraj[ptIt];
      for(const auto& child: subChildren)
      {
        AppendTrajectory(viewer, nextID, iter, child, parentToTraj, services);
      }
    }
  }

  REGISTER_PLUGIN(LinearTraj, EventDrawer);
}
