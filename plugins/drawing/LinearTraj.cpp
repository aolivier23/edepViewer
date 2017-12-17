//File: LinearTraj.cpp
//Brief: A plugin that draws the ROOT geometry for the edepsim display.  Takes a TGeoManager as drawing data and 
//       draws 3D shapes using ROOT's tesselation facilities.  
//Author: Andrew Olivier aolivier@ur.rochester.edu

//draw includes
#include "plugins/drawing/LinearTraj.h"

//gl includes
#include "gl/model/Path.h"
#include "gl/model/Point.h"

//plugin factory for macro
#include "plugins/Factory.cpp"

//ROOT includes
#include "TGeoManager.h" 
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
    const auto proc = pt.Process;
    const auto subProc = pt.Subprocess;
                                                                                                    
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
  LinearTraj::LinearTraj(const tinyxml2::XMLElement* config): FiducialDrawer(config)
  {
    fPointRad = config->FloatAttribute("PointRad", 0.010);
    fLineWidth = config->FloatAttribute("LineWidth", 0.008);
  }

  void LinearTraj::doRequestScenes(mygl::Viewer& viewer) 
  {
    //Configure trajectory Scene
    auto& trajTree = viewer.MakeScene("Trajectories", fTrajRecord, "/home/aolivier/app/evd/src/gl/shaders/colorPerVertex.frag", "/home/aolivier/app/evd/src/gl/shaders/colorPerVertex.vert", "/home/aolivier/app/evd/src/gl/shaders/wideLine.geom");
    trajTree.append_column("Particle Type", fTrajRecord.fPartName);
    //trajTree.insert_column_with_data_func(-1, "Particle", fPartNameRender, sigc::mem_fun(*this, &EvdWindow::ColToColor));
    trajTree.append_column("KE [MeV]", fTrajRecord.fEnergy);

    //Configure Trajectory Point Scene
    auto& ptTree = viewer.MakeScene("TrajPts", fTrajPtRecord, "/home/aolivier/app/evd/src/gl/shaders/colorPerVertex.frag", "/home/aolivier/app/evd/src/gl/shaders/colorPerVertex.vert", "/home/aolivier/app/evd/src/gl/shaders/widePoint.geom");
    ptTree.append_column("Particle Type", fTrajPtRecord.fParticle);
    ptTree.append_column("Process", fTrajPtRecord.fProcess);
    ptTree.append_column("Momentum [MeV/c]", fTrajPtRecord.fMomMag);
    ptTree.append_column("Time", fTrajPtRecord.fTime);
  }

  void LinearTraj::doDrawEvent(const TG4Event& evt, const TGeoManager& man, mygl::Viewer& viewer, mygl::VisID& nextID, Services& services) 
  {
    //First, clear the scenes I plan to draw on
    viewer.GetScenes().find("Trajectories")->second.RemoveAll();
    viewer.GetScenes().find("TrajPts")->second.RemoveAll();

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
      auto row = *(viewer.GetScenes().find("Trajectories")->second.NewTopLevelNode());
        
      //Turn GENIE's interaction string into something easier to read
      std::smatch match;
      if(!std::regex_match(prim.Reaction, match, genieToEvd)) std::cerr << "Got interaction string from GENIE that does not match what I expect:\n"
                                                                      << prim.Reaction << "\n";

      const std::string nu = TDatabasePDG::Instance()->GetParticle(std::stoi(match[1].str()))->GetName();
      //const std::string nucleus = fPdgDB.GetParticle(match[2].str().c_str())->GetName(); //TODO: TDatabasPDG can't read PDG codes for nuclei

      const std::string nucleon = TDatabasePDG::Instance()->GetParticle(std::stoi(match[3].str()))->GetName(); 
      row[fTrajRecord.fPartName] = nu+" "+match[5].str()+" "+match[6].str();//+" on "/*+nucleus+" "*/+nucleon;
      row[fTrajRecord.fEnergy] = -1; //TODO: Use std::regex (maybe) to extract this from prim.Reaction

      //Add this interaction vertex to the scene of trajectory points
      const auto ptPos = prim.Position;
      const int pdg = std::stoi(match[1].str());
      const auto color = (*(services.fPDGToColor))[pdg];

      //TODO: Function in Scene/Viewer to add a new Drawable with a new top-level TreeRow
      auto ptRow = viewer.AddDrawable<mygl::Point>("TrajPts", nextID++, 
                                                    *(viewer.GetScenes().find("TrajPts")->second.NewTopLevelNode()), true, 
                                                    glm::mat4(), glm::vec3(ptPos.X(), ptPos.Y(), ptPos.Z()), glm::vec4(color, 1.0), fPointRad);
      ptRow[fTrajPtRecord.fMomMag] = -1.; //TODO: Get primary momentum
      ptRow[fTrajPtRecord.fTime] = ptPos.T();
      ptRow[fTrajPtRecord.fProcess] = nu+" "+match[5].str()+" "+match[6].str();
      ptRow[fTrajPtRecord.fParticle] = nu;

      const auto& children = parentID[-1];
      for(const auto& child: children) AppendTrajectory(viewer, man, nextID, row, child, parentID, ptRow, services);
    }
  }

  //Helper functions for drawing trajectories and trajectory points
  void LinearTraj::AppendTrajectory(mygl::Viewer& viewer, const TGeoManager& man, mygl::VisID& nextID, const Gtk::TreeModel::Row& parent, 
                                    const TG4Trajectory& traj, std::map<int, std::vector<TG4Trajectory>>& parentToTraj, 
                                    const Gtk::TreeModel::Row& parentPt, Services& services)
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
      if(IsFiducial(pos.Vect(), man))
      {
        vertices.emplace_back(pos.X(), pos.Y(), pos.Z());
      }
      else if(pointIt != points.begin()) //Extrapolate to the face of the fiducial volume
      {
        const auto prevPoint = (pointIt-1)->Position;

        //Make sure previous point was in the fiducial volume
        if(IsFiducial(prevPoint.Vect(), man))
        {
          //TGeoShape::DistFromInside needs two 3-vectors in the local frame:
          //The direction
          const auto dirLocalV = InLocal((pos-prevPoint).Vect().Unit(), man);
          double dirLocal[] = {dirLocalV.X(), dirLocalV.Y(), dirLocalV.Z()};

          //The position inside the volume
          const auto prevLocal = InLocal(prevPoint.Vect(), man);          
          double local[] = {prevLocal.X(), prevLocal.Y(), prevLocal.Z()};

          const auto dist = GetFiducial().GetVolume()->GetShape()->DistFromInside(local, dirLocal, 3); 

          //Now, convert the position of the point to draw back to the master frame in which everything is drawn          
          const auto master = InMaster(TVector3(local[0]+dirLocal[0]*dist, local[1]+dirLocal[1]*dist, local[2]+dirLocal[2]*dist), man);
          vertices.emplace_back(master.X(), master.Y(), master.Z());
        }
      }
    }

    //TODO: Change "false" back to "true" to draw trajectories by default
    auto row = viewer.AddDrawable<mygl::Path>("Trajectories", nextID, parent, true, glm::mat4(), vertices, glm::vec4((glm::vec3)color, 1.0), fLineWidth); 
    row[fTrajRecord.fPartName] = traj.Name;
    auto p = traj.InitialMomentum;
    const double invariantMass = std::sqrt((p.Mag2()>0)?p.Mag2():0); //Make sure the invariant mass is > 0 as it should be.  It might be negative for 
                                                                       //photons because of floating point precision behavior.  Never trust a computer to 
                                                                       //calculate 0...
    row[fTrajRecord.fEnergy] = p.E()-invariantMass; //Kinetic energy
    Gdk::RGBA gdkColor;
    gdkColor.set_rgba(color.r, color.g, color.b, 1.0);
    ++nextID;

    //TODO: Append children based on what points they start at
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
      auto ptRow = AddTrajPt(viewer, man, nextID, traj.Name, point, parentPt, glm::vec4(color, 1.0));
      const auto& subChildren = ptToTraj[ptIt];
      for(const auto& child: subChildren)
      {
        AppendTrajectory(viewer, man, nextID, row, child, parentToTraj, ptRow, services);
      }
    }
  }

  Gtk::TreeModel::Row LinearTraj::AddTrajPt(mygl::Viewer& viewer, const TGeoManager& man, mygl::VisID& nextID, const std::string& particle, 
                                            const TG4TrajectoryPoint& pt, const Gtk::TreeModel::Row& parent, const glm::vec4& color)
  {
    //Add Trajectory Point
    const auto pos = pt.Position;
    auto ptRow = viewer.AddDrawable<mygl::Point>("TrajPts", nextID++,
                                                  parent, true,
                                                  glm::mat4(), glm::vec3(pos.X(), pos.Y(), pos.Z()), color, fPointRad);
    ptRow[fTrajPtRecord.fMomMag] = pt.Momentum.Mag();
    ptRow[fTrajPtRecord.fTime] = pos.T();
    ptRow[fTrajPtRecord.fProcess] = ::ProcStr(pt); //TODO: Convert Geant process codes to string
    ptRow[fTrajPtRecord.fParticle] = particle;
  
    return ptRow;
  }

  REGISTER_PLUGIN(LinearTraj, EventDrawer);
}
