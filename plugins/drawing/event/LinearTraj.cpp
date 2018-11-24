//File: LinearTraj.cpp
//Brief: A plugin that draws true trajectories of particles.  
//Author: Andrew Olivier aolivier@ur.rochester.edu

//draw includes
#include "LinearTraj.h"

//gl includes
#include "gl/model/Path.h"

//edepsim includes
#include "TG4Event.h" 

//ROOT includes
#include "TDatabasePDG.h"

//c++ includes
#include <regex>

namespace draw
{
  LinearTraj::LinearTraj(const YAML::Node& config): fLineWidth(0.008), fDefaultDraw(false), fTrajRecord(new TrajRecord())
  {
    if(config["LineWidth"]) fLineWidth = config["LineWidth"].as<float>();
    if(config["DefaultDraw"]) fDefaultDraw = config["DefaultDraw"].as<float>();
  }

  legacy::scene_t& LinearTraj::doRequestScene(mygl::Viewer& viewer) 
  {
    //Configure trajectory Scene
    return viewer.MakeScene("Trajectories", fTrajRecord, INSTALL_GLSL_DIR "/colorPerVertex.frag", INSTALL_GLSL_DIR "/colorPerVertex.vert", INSTALL_GLSL_DIR "/wideLine.geom");
  }

  std::unique_ptr<legacy::model_t> LinearTraj::doDraw(const TG4Event& evt, Services& services) 
  {
    auto model = std::make_unique<legacy::model_t>(fTrajRecord);
    auto& trajScene = *model;

    //Next, make maps of trackID to particle and parent ID to particle
    std::map<int, std::vector<TG4Trajectory>> parentID;
    for(auto& traj: evt.Trajectories)
    {
      #ifdef EDEPSIM_FORCE_PRIVATE_FIELDS
      parentID[traj.GetParentId()].push_back(traj);
      #else
      parentID[traj.ParentId].push_back(traj);
      #endif
    }

    //Then, add particles to list tree and viewer
    std::regex genieToEvd(R"(nu:(-?[[:digit:]]+);tgt:([[:digit:]]+);N:([[:digit:]]+)(.*);proc:Weak\[([[:alpha:]]+)\],([[:alpha:]]+);(.*))");
    auto& primaries = evt.Primaries;
    for(auto& prim: primaries)
    {
      auto row = trajScene.emplace(fDefaultDraw);
        
      //Turn GENIE's interaction string into something easier to read
      std::smatch match;
      std::string nu;
      //int pdg;
       
      #ifdef EDEPSIM_FORCE_PRIVATE_FIELDS
      const std::string reaction = prim.GetReaction();
      #else
      const std::string reaction = prim.Reaction;
      #endif
      if(std::regex_match(reaction, match, genieToEvd)) 
      {

        const std::string nu = TDatabasePDG::Instance()->GetParticle(std::stoi(match[1].str()))->GetName();
        //const std::string nucleus = fPdgDB.GetParticle(match[2].str().c_str())->GetName(); //TODO: TDatabasPDG can't read PDG codes for nuclei

        /*const auto particle = TDatabasePDG::Instance()->GetParticle(std::stoi(match[3].str()));
        if(particle)
        {
          const std::string nucleon = particle->GetName(); 
        }*/

        //Add this interaction vertex to the scene of trajectory points
        //pdg = std::stoi(match[1].str());
      }
      else
      {
        std::cerr << "WARNING: Got interaction string from GENIE that does not match what I expect:\n"
                  << reaction << "\n";
        nu = reaction;
        //pdg = 0; //Supposed to be a "Geantino" to indicate that something is wrong
      }

      row[fTrajRecord->fPartName] = nu+" "+match[5].str()+" "+match[6].str();//+" on "/*+nucleus+" "*/+nucleon;
      row[fTrajRecord->fEnergy] = -1; //TODO: Get this from updated TG4PrimaryVertex?

      const auto& children = parentID[-1];
      for(const auto& child: children) AppendTrajectory(row, child, parentID, services);
    }
    return model;
  }

  //Helper functions for drawing trajectories and trajectory points
  void LinearTraj::AppendTrajectory(legacy::model_t::view& parent, const TG4Trajectory& traj, 
                                    std::map<int, std::vector<TG4Trajectory>>& parentToTraj, Services& services)
  {
    #ifdef EDEPSIM_FORCE_PRIVATE_FIELDS
    const int pdg = traj.GetPDGCode();
    #else
    const int pdg = traj.PDGCode;
    #endif
    const auto color = (*(services.fPDGToColor))[pdg];

    auto points = traj.Points;
    std::vector<glm::vec3> vertices;
    for(auto pointIt = points.begin(); pointIt != points.end(); ++pointIt)
    {
      auto& point = *pointIt;
      #ifdef EDEPSIM_FORCE_PRIVATE_FIELDS
      const auto pos = point.GetPosition();
      #else
      const auto pos = point.Position;
      #endif

      //Require that point is inside fiducial volume
      if(services.fGeometry->IsFiducial(pos.Vect()))
      {
        vertices.emplace_back(pos.X(), pos.Y(), pos.Z());
      }
      else if(pointIt != points.begin()) //Extrapolate to the face of the fiducial volume
      {
        #ifdef EDEPSIM_FORCE_PRIVATE_FIELDS
        const auto prevPoint = (pointIt-1)->GetPosition();
        #else
        const auto prevPoint = (pointIt-1)->Position;
        #endif

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

    auto row = parent.emplace<mygl::Path>(true, glm::mat4(), vertices, 
                                          glm::vec4((glm::vec3)color, 1.0), fLineWidth); 
    #ifdef EDEPSIM_FORCE_PRIVATE_FIELDS
    row[fTrajRecord->fPartName] = traj.GetName();
    auto p = traj.GetInitialMomentum();
    #else
    row[fTrajRecord->fPartName] = traj.Name;
    auto p = traj.InitialMomentum;
    #endif
    const double invariantMass = std::sqrt((p.Mag2()>0)?p.Mag2():0); //Make sure the invariant mass is > 0 as it should be.  It might be negative for 
                                                                       //photons because of floating point precision behavior.  Never trust a computer to 
                                                                       //calculate 0...
    row[fTrajRecord->fEnergy] = p.E()-invariantMass; //Kinetic energy

    //Second pass over trajectory points to find starting points of children.
    #ifdef EDEPSIM_FORCE_PRIVATE_FIELDS
    auto children = parentToTraj[traj.GetTrackId()];
    #else
    auto children = parentToTraj[traj.TrackId];
    #endif

    //Produce map of closest trajectory point to child trajectory
    std::map<std::vector<TG4TrajectoryPoint>::iterator, std::vector<TG4Trajectory>> ptToTraj;
    for(const auto& child: children)
    {
      ptToTraj[std::min_element(points.begin(), points.end(), [&child](const TG4TrajectoryPoint& first, 
                                                                       const TG4TrajectoryPoint& second)
                                                              {
                                                                #ifdef EDEPSIM_FORCE_PRIVATE_FIELDS
                                                                return (first.GetPosition() - child.Points.front().GetPosition()).Mag() < 
                                                                       (second.GetPosition() - child.Points.front().GetPosition()).Mag();
                                                                #else
                                                                return (first.Position - child.Points.front().Position).Mag() <
                                                                       (second.Position - child.Points.front().Position).Mag();
                                                                #endif
                                                              })].push_back(child);
    } 
      
    for(auto ptIt = points.begin(); ptIt != points.end(); ++ptIt)
    {
      const auto& subChildren = ptToTraj[ptIt];
      for(const auto& child: subChildren)
      {
        AppendTrajectory(row, child, parentToTraj, services);
      }
    }
  }

  REGISTER_EVENT(LinearTraj);
}
