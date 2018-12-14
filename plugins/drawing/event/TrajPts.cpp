//File: TrajPts.cpp
//Brief: A plugin that draws true trajectories of particles.  
//Author: Andrew Olivier aolivier@ur.rochester.edu

//draw includes
#include "TrajPts.h"

//gl includes
#include "gl/model/Path.h"
#include "gl/model/Point.h"

//plugin factory for macro
#include "plugins/Factory.cpp"

//ROOT includes
#include "TDatabasePDG.h"

//edepsim includes
#include "TG4Event.h" 

//tinyxml2 include for configuration
#include <tinyxml2.h>

//c++ includes
#include <regex>

namespace
{
  //TODO: This function seems generally useful for working with edepsim.  Move it to its' own file.
  std::string ProcStr(const TG4TrajectoryPoint& pt)
  {
    #ifdef EDEPSIM_FORCE_PRIVATE_FIELDS
    const auto proc = pt.GetProcess();
    const auto subProc = pt.GetSubprocess();
    #else
    const auto proc = pt.Process;
    const auto subProc = pt.Subprocess;
    #endif
                                                                                                    
    std::string procStr = "";
    //Process Type
    if(proc == 0)      procStr = "Not Defined";
    else if(proc == 1) procStr = "Transportation";
    else if(proc == 2) procStr = "Electromagnetic";
    else if(proc == 3) procStr = "Optical";
    else if(proc == 4) procStr = "Hadronic";
    else if(proc == 5) procStr = "PhotoLeptonHadron";
    else if(proc == 6) procStr = "Decay";
    else if(proc == 7) procStr = "General";
    else if(proc == 8) procStr = "Parameterization";
    else if(proc == 9) procStr = "User Defined";
    else procStr = "Unknown";
                                                                                                    
    procStr += " ";
                                                                                                    
    //Subtype
    if(subProc == 1)        procStr += "Coulomb Scattering";
    else if(subProc == 2)   procStr += "Ionization";
    else if(subProc == 3)   procStr += "Bremsstrahlung";
    else if(subProc == 4)   procStr += "Pair Production";
    else if(subProc == 8)   procStr += "Nuclear Stopping";
                                                                                                    
    else if(subProc == 10)  procStr += "Multiple Scattering";
    else if(subProc == 12)  procStr += "Photoelectric";
    else if(subProc == 13)  procStr += "Compton Scattering";
    else if(subProc == 14)  procStr += "Gamma Conversion";
                                                                                                    
    else if(subProc == 111) procStr += "Elastic";
    else if(subProc == 121) procStr += "Inelastic";
    else if(subProc == 131) procStr += "Capture";
    else if(subProc == 161) procStr += "Charge Exchange";
                                                                                                    
    else if(subProc == 401) procStr += "General Step Limit";
    else                    procStr += "Unknown";
                                                                                                    
    return procStr;
  }
}

namespace draw
{
  TrajPts::TrajPts(const YAML::Node& config): fPointRad(0.010), fDefaultDraw(true), fTrajPtRecord(new TrajPtRecord)
  {
    if(config["PointRad"]) fPointRad = config["PointRad"].as<float>();
    if(config["DefaultDraw"]) fDefaultDraw = config["DefaultDraw"].as<bool>();
  }

  legacy::scene_t& TrajPts::doRequestScene(mygl::Viewer& viewer) 
  {
    //Configure Trajectory Point Scene
    return viewer.MakeScene("TrajPts", fTrajPtRecord, INSTALL_GLSL_DIR "/colorPerVertex.frag", INSTALL_GLSL_DIR "/colorPerVertex.vert", INSTALL_GLSL_DIR "/widePoint.geom");
  }

  std::unique_ptr<legacy::model_t> TrajPts::doDraw(const TG4Event& evt, Services& services) 
  {
    //Get the TrajPt Scene and remove trajectory points from last event
    auto model = std::make_unique<legacy::model_t>(fTrajPtRecord);
    auto& ptScene = *model;

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
      //Turn GENIE's interaction string into something easier to read
      std::smatch match;
      #ifdef EDEPSIM_FORCE_PRIVATE_FIELDS
      const std::string reaction = prim.GetReaction();
      #else
      const std::string reaction = prim.Reaction;
      #endif
      
      if(!std::regex_match(reaction, match, genieToEvd)) std::cerr << "Got interaction string from GENIE that does not match what I expect:\n"
                                                                   << reaction << "\n";

      const std::string nu = TDatabasePDG::Instance()->GetParticle(std::stoi(match[1].str()))->GetName();
      //const std::string nucleus = fPdgDB.GetParticle(match[2].str().c_str())->GetName(); //TODO: TDatabasPDG can't read PDG codes for nuclei

      /*const auto particle = TDatabasePDG::Instance()->GetParticle(std::stoi(match[3].str()));
      if(particle)
      {
        const std::string nucleon = particle->GetName(); 
      }*/

      //Add this interaction vertex to the scene of trajectory points
      #ifdef EDEPSIM_FORCE_PRIVATE_FIELDS
      const auto ptPos = prim.GetPosition();
      #else
      const auto ptPos = prim.Position;
      #endif
      const int pdg = std::stoi(match[1].str());
      const auto color = (*(services.fPDGToColor))[pdg];

      //TODO: Function in Scene/Viewer to add a new Drawable with a new top-level TreeRow
      auto ptRow = ptScene.emplace(fDefaultDraw).emplace<mygl::Point>(true, glm::mat4(), glm::vec3(ptPos.X(), 
                                                                      ptPos.Y(), ptPos.Z()), glm::vec4(color, 1.0), fPointRad);
      ptRow[fTrajPtRecord->fMomMag] = -1.; //TODO: Get primary momentum
      ptRow[fTrajPtRecord->fTime] = ptPos.T();
      ptRow[fTrajPtRecord->fProcess] = nu+" "+match[5].str()+" "+match[6].str();
      ptRow[fTrajPtRecord->fParticle] = nu;

      const auto& children = parentID[-1];
      for(const auto& child: children) AppendTrajPts(ptRow, child, parentID, services);
    }
    return model;
  }

  //Helper functions for drawing trajectories and trajectory points
  void TrajPts::AppendTrajPts(legacy::model_t::view& parent, const TG4Trajectory& traj, 
                              std::map<int, std::vector<TG4Trajectory>>& parentToTraj, Services& services)
  {
    #ifdef EDEPSIM_FORCE_PRIVATE_FIELDS
    const int pdg = traj.GetPDGCode();
    #else
    const int pdg = traj.PDGCode;
    #endif
    const auto color = (*(services.fPDGToColor))[pdg];
    auto points = traj.Points;

    //Second pass over trajectory points to find starting points of children.
    #ifdef EDEPSIM_FORCE_PRIVATE_FIELDS
    const auto id = traj.GetTrackId();
    #else
    const auto id = traj.TrackId;
    #endif
    auto children = parentToTraj[id];

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
      const auto& point = *ptIt;
      #ifdef EDEPSIM_FORCE_PRIVATE_FIELDS
      const std::string name = traj.GetName();
      #else
      const std::string name = traj.Name;
      #endif
      auto ptRow = AddTrajPt(parent, name, point, glm::vec4(color, 1.0));
      const auto& subChildren = ptToTraj[ptIt];
      for(const auto& child: subChildren)
      {
        AppendTrajPts(ptRow, child, parentToTraj, services);
      }
    }
  }

  legacy::model_t::view TrajPts::AddTrajPt(legacy::model_t::view& parent, const std::string& particle, 
                                           const TG4TrajectoryPoint& pt, const glm::vec4& color)
  {
    //Add Trajectory Point
    #ifdef EDEPSIM_FORCE_PRIVATE_FIELDS
    const auto pos = pt.GetPosition();
    #else
    const auto pos = pt.Position;
    #endif
    auto ptRow = parent.emplace<mygl::Point>(true, glm::mat4(), glm::vec3(pos.X(), pos.Y(), pos.Z()), color, fPointRad);

    #ifdef EDEPSIM_FORCE_PRIVATE_FIELDS
    ptRow[fTrajPtRecord->fMomMag] = pt.GetMomentum().Mag();
    #else
    ptRow[fTrajPtRecord->fMomMag] = pt.Momentum.Mag();
    #endif
    ptRow[fTrajPtRecord->fTime] = pos.T();
    ptRow[fTrajPtRecord->fProcess] = ::ProcStr(pt); //TODO: Convert Geant process codes to string
    ptRow[fTrajPtRecord->fParticle] = particle;
  
    return ptRow;
  }

  REGISTER_EVENT(TrajPts);
}
