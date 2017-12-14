//File: EvdWindow.cpp
//Brief: Displays a ROOT geometry with a List Tree view that shows data about each element.
//       Based heavily on https://developer.gnome.org/gtkmm-tutorial/stable/sec-treeview-examples.html.en
//Author: Andrew Olivier

//evd includes
#include "EvdWindow.h"

//gl includes
#include "gl/model/PolyMesh.h"
#include "gl/model/Path.h"
#include "gl/model/Grid.h"
#include "gl/model/Point.h"
#include "gl/camera/PlaneCam.h"

//TODO: Replace this with a factory
#include "plugins/drawing/GeoDrawer.h"

//glm includes
#include <glm/gtc/type_ptr.hpp>

//ROOT includes
#include "TGeoManager.h"
#include "TGeoNode.h"
#include "TGeoVolume.h"
#include "TFile.h"
#include "TLorentzVector.h"
#include "TBuffer3D.h"
#include "TDatabasePDG.h"
#include "TParticlePDG.h"

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

namespace mygl
{
  EvdWindow::EvdWindow(const std::string& fileName, const bool darkColors): Gtk::Window(), 
    fViewer(std::unique_ptr<mygl::Camera>(new mygl::PlaneCam(glm::vec3(0., 0., 1000.), glm::vec3(0.0, 1.0, 0.0), 10000., 100.)), 
            darkColors?Gdk::RGBA("(0.f, 0.f, 0.f)"):Gdk::RGBA("(1.f, 1.f, 1.f)"), 10., 10., 10.), 
    fVBox(Gtk::ORIENTATION_VERTICAL), fNavBar(), fPrint("Print"), fNext("Next"), fReload("Reload"), fEvtNumWrap(), fEvtNum(), fFileChoose("File"), fFileLabel(fileName),
    fFileName(fileName), fNextID(0, 0, 0), fPDGColor(), fPDGToColor(), fPdgDB(),
    fPalette(0., 8.), fLineWidth(0.008), fPointRad(0.010)
  {
    set_title("edepsim Display Window");
    set_border_width(5);
    set_default_size(1400, 1000);

    build_toolbar();
    add(fVBox);
    fVBox.pack_start(fNavBar, Gtk::PACK_SHRINK);
    fVBox.pack_end(fViewer);

    fViewer.signal_map().connect(sigc::mem_fun(*this, &EvdWindow::make_scenes)); //Because signal_realize is apparently emitted before this object's 
                                                                                 //children are realize()d


    //TODO: Load and configure as a plugin
    fGlobalDrawers.emplace_back(new draw::GeoDrawer());
    
    show_all_children();
  }

  EvdWindow::~EvdWindow() {}

  void EvdWindow::SetFile(const std::string& fileName)
  {
    fFile.reset(new TFile(fileName.c_str()));
    fReader.reset(new TTreeReader("EDepSimEvents", fFile.get()));
    fCurrentEvt.reset(new TTreeReaderValue<TG4Event>(*fReader, "Event"));
    fReader->Next(); //TODO: replace with a dedicated event controller
    ReadGeo(); 
    ReadEvent();
  }

  void EvdWindow::ReadGeo()
  {
    fNextID = mygl::VisID();
    
    fGeoManager = (TGeoManager*)(fFile->Get("EDepSimGeometry"));
    auto man = fGeoManager;
    if(man != nullptr)
    {
      for(const auto& drawPtr: fGlobalDrawers) drawPtr->DrawEvent(*man, fViewer, fNextID);

      //Make sure a sensible fiducial node is set
      fFiducialName.set_text(man->GetTopNode()->GetName());
      set_fiducial();
      std::cout << "Done generating the geometry.\n";
    } //TODO: else throw exception
    else std::cerr << "Failed to read geometry manager from file.\n";
  }

  void EvdWindow::ReadEvent()
  { 
    //First, remove the previous event from all scenes except geometry
    //TODO: Let Drawer do this
    for(auto& scenePair: fViewer.GetScenes())
    {
      if(scenePair.first != "Geometry" && scenePair.first != "Guides") scenePair.second.RemoveAll();
    }

    //Next, make maps of trackID to particle and parent ID to particle
    std::map<int, std::vector<TG4Trajectory>> parentID;

    for(auto& traj: (*fCurrentEvt)->Trajectories)
    {
      parentID[traj.ParentId].push_back(traj);
    }

    //Then, add particles to list tree and viewer
    std::regex genieToEvd(R"(nu:(-?[[:digit:]]+);tgt:([[:digit:]]+);N:([[:digit:]]+)(.*);proc:Weak\[([[:alpha:]]+)\],([[:alpha:]]+);(.*))");
    auto& primaries = (*fCurrentEvt)->Primaries;
    for(auto& prim: primaries)
    {
      auto row = *(fViewer.GetScenes().find("Trajectories")->second.NewTopLevelNode());
      
      //Turn GENIE's interaction string into something easier to read
      std::smatch match;
      if(!std::regex_match(prim.Reaction, match, genieToEvd)) std::cerr << "Got interaction string from GENIE that does not match what I expect:\n"
                                                                        << prim.Reaction << "\n";

      const std::string nu = fPdgDB.GetParticle(std::stoi(match[1].str()))->GetName();
      //const std::string nucleus = fPdgDB.GetParticle(match[2].str().c_str())->GetName(); //TODO: TDatabasPDG can't read PDG codes for nuclei
      const std::string nucleon = fPdgDB.GetParticle(std::stoi(match[3].str()))->GetName(); 
      row[fTrajRecord.fPartName] = nu+" "+match[5].str()+" "+match[6].str();//+" on "/*+nucleus+" "*/+nucleon;
      row[fTrajRecord.fEnergy] = -1; //TODO: Use std::regex (maybe) to extract this from prim.Reaction
      row[fTrajRecord.fColor] = Gdk::RGBA("(0., 0., 0.)");

      //Add this interaction vertex to the scene of trajectory points
      const auto ptPos = prim.Position;
      const int pdg = std::stoi(match[1].str());
      if(fPDGToColor.find(pdg) == fPDGToColor.end()) fPDGToColor[pdg] = fPDGColor++;
      const auto color = fPDGToColor[pdg];

      //TODO: Function in Scene/Viewer to add a new Drawable with a new top-level TreeRow
      auto ptRow = fViewer.AddDrawable<mygl::Point>("TrajPts", fNextID++, 
                                                    *(fViewer.GetScenes().find("TrajPts")->second.NewTopLevelNode()), true, 
                                                    glm::mat4(), glm::vec3(ptPos.X(), ptPos.Y(), ptPos.Z()), glm::vec4(color, 1.0), fPointRad);
      ptRow[fTrajPtRecord.fMomMag] = -1.; //TODO: Get primary momentum
      ptRow[fTrajPtRecord.fTime] = ptPos.T();
      ptRow[fTrajPtRecord.fProcess] = nu+" "+match[5].str()+" "+match[6].str();
      ptRow[fTrajPtRecord.fParticle] = nu;

      const auto& children = parentID[-1];
      for(const auto& child: children) AppendTrajectory(row, child, parentID, ptRow);
    }

    //Last, set current event number for GUI
    fEvtNum.set_text(std::to_string(fReader->GetCurrentEntry()));
    fViewer.GetScenes().find("Trajectories")->second.fTreeView.expand_to_path(Gtk::TreePath("0")); //TODO: Move this to the Viewer?

    //Draw true energy deposits color-coded by energy
    auto edepToDet = (*fCurrentEvt)->SegmentDetectors; //A map from sensitive volume to energy deposition

    for(auto& det: edepToDet)
    {
      auto detName = det.first;
      auto detRow = *(fViewer.GetScenes().find("EDep")->second.NewTopLevelNode());
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
      /*double mindEdx = 0., maxdEdx = 1e6; //, sumdEdx = 0.; //TODO: Check average dEdx and look at order of magnitude of its' ratio with 
                                                            //      maxdEdx to decide whether to use log scale.
      for(auto& edep: edeps)
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
        auto mat = fGeoManager->FindNode(mid.X(), mid.Y(), mid.Z())->GetVolume()->GetMaterial();
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
          auto mat = fGeoManager->FindNode(pos.X(), pos.Y(), pos.Z())->GetVolume()->GetMaterial();
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
        
        auto row = fViewer.AddDrawable<mygl::Path>("EDep", fNextID, detRow, true, glm::mat4(), std::vector<glm::vec3>{firstPos, lastPos}, glm::vec4(fPalette(dEdx), 1.0), fLineWidth);
        //fPalette(dEdx), 1.0));
        //fPDGToColor[(*fCurrentEvt)->Trajectories[edep.PrimaryId].PDGCode], 1.0));
        //palette(std::log10(dEdx)), 1.0));
        //fPalette(std::log10(energy)), 1.0));
        row[fEDepRecord.fScintE]  = edep.SecondaryDeposit;
        row[fEDepRecord.fEnergy]  = energy;
        row[fEDepRecord.fdEdx]    = dEdx;
        row[fEDepRecord.fT0]      = start.T();
        row[fEDepRecord.fPrimName] = (*fCurrentEvt)->Trajectories[edep.PrimaryId].Name; //TODO: energy depositions children of contributing tracks?
        ++fNextID;

        sumE += energy;
        sumScintE += edep.SecondaryDeposit;
        if(start.T() < minT) minT = start.T();
      }

      //Fill values for this sensitive detector with cumulative energy from its' hits and time of earliest hit
      detRow[fEDepRecord.fEnergy] = sumE;
      detRow[fEDepRecord.fScintE] = sumScintE;
      detRow[fEDepRecord.fT0]     = minT;
      detRow[fEDepRecord.fdEdx]   = 0.; //TODO: Come up with a more meaningful value for this.
    }
  }
  
  //Function to draw any guides the user requests, like axes or a scale.  Starting with a one meter scale at the 
  //center of the screen broken into mm for now.  
  void EvdWindow::DrawGuides() 
  {
    auto root = *(fViewer.GetScenes().find("Guides")->second.NewTopLevelNode());
    root[fGuideRecord.fName] = "Measurement Objects";
  
    const double gridSize = 1e5;
    //A 1m grid 
    auto row = fViewer.AddDrawable<mygl::Grid>("Guides", fNextID++, root, false, glm::mat4(), gridSize, 1000., gridSize, 1000., 
                                               glm::vec4(0.3f, 0.0f, 0.9f, 0.2f), fLineWidth);
    row[fGuideRecord.fName] = "1m Grid";

    //A 1dm grid 
    row = fViewer.AddDrawable<mygl::Grid>("Guides", fNextID++, root, false, glm::mat4(), gridSize, 100., gridSize, 100.,
                                               glm::vec4(0.3f, 0.0f, 0.9f, 0.2f), fLineWidth);
    row[fGuideRecord.fName] = "1dm Grid";

    //A 1cm grid
    row = fViewer.AddDrawable<mygl::Grid>("Guides", fNextID++, root, false, glm::mat4(), gridSize, 10., gridSize, 10.,
                                               glm::vec4(0.3f, 0.0f, 0.9f, 0.2f), fLineWidth);
    row[fGuideRecord.fName] = "1cm Grid";

    //A 1mm grid
    row = fViewer.AddDrawable<mygl::Grid>("Guides", fNextID++, root, false, glm::mat4(), gridSize, 1., gridSize, 1.,
                                               glm::vec4(0.3f, 0.0f, 0.9f, 0.2f), fLineWidth);
    row[fGuideRecord.fName] = "1mm Grid";
    
    //TODO: Smaller grids, maybe with dashed lines
    //TODO: Drawing the 1mm grid is very slow.  
  }

  Gtk::TreeModel::Row EvdWindow::AddTrajPt(const std::string& particle, const TG4TrajectoryPoint& pt, const Gtk::TreeModel::Row& parent, 
                                           const glm::vec4& color)
  {
    //Add Trajectory Point
    const auto pos = pt.Position;
    auto ptRow = fViewer.AddDrawable<mygl::Point>("TrajPts", fNextID++,
                                                  parent, true,
                                                  glm::mat4(), glm::vec3(pos.X(), pos.Y(), pos.Z()), color, fPointRad);
    ptRow[fTrajPtRecord.fMomMag] = pt.Momentum.Mag(); 
    ptRow[fTrajPtRecord.fTime] = pos.T();
    ptRow[fTrajPtRecord.fProcess] = ::ProcStr(pt); //TODO: Convert Geant process codes to string
    ptRow[fTrajPtRecord.fParticle] = particle;

    return ptRow;
  }

  void EvdWindow::AppendTrajectory(const Gtk::TreeModel::Row& parent, const TG4Trajectory& traj, 
                                   std::map<int, std::vector<TG4Trajectory>>& parentToTraj, 
                                   const Gtk::TreeModel::Row& parentPt)
  {
    const int pdg = traj.PDGCode;
    //TODO: Legend
    if(fPDGToColor.find(pdg) == fPDGToColor.end()) fPDGToColor[pdg] = fPDGColor++;
    auto color = fPDGToColor[pdg];

    auto points = traj.Points;
    std::vector<glm::vec3> vertices;
    for(auto pointIt = points.begin(); pointIt != points.end(); ++pointIt)
    {
      auto& point = *pointIt;
      const auto& pos = point.Position;

      //Require that point is inside fiducial volume
      double master[] = {pos.X(), pos.Y(), pos.Z()};
      double local[3] = {}; 
      fFiducialMatrix->MasterToLocal(master, local);

      if(fFiducialNode->GetVolume()->Contains(local)) 
      {
        vertices.emplace_back(pos.X(), pos.Y(), pos.Z());
      }
      else if(pointIt != points.begin()) //Extrapolate to the face of the fiducial volume
      {
        //Make sure previous point was in the fiducial volume
        const auto& prevPoint = (pointIt-1)->Position;
        master[0] = prevPoint.X();
        master[1] = prevPoint.Y();
        master[2] = prevPoint.Z();
        fFiducialMatrix->MasterToLocal(master, local);
        if(fFiducialNode->GetVolume()->Contains(local))
        {
          //TGeoShape::DistFromInside needs two 3-vectors in the local frame:
          //The direction
          const auto dirVec = (pos-prevPoint).Vect().Unit();
          double dir[] = {dirVec.X(), dirVec.Y(), dirVec.Z()};
          double dirLocal[3] = {};
          fFiducialMatrix->MasterToLocalVect(dir, dirLocal);

          //The position inside the volume
          double prevMaster[] = {prevPoint.X(), prevPoint.Y(), prevPoint.Z()};
          fFiducialMatrix->MasterToLocal(prevMaster, local);
  
          const auto dist = fFiducialNode->GetVolume()->GetShape()->DistFromInside(local, dirLocal, 3); 
  
          //Now, convert the position of the point to draw back to the master frame in which everything is drawn          
          double newPtLocal[] = {local[0]+dirLocal[0]*dist, local[1]+dirLocal[1]*dist, local[2]+dirLocal[2]*dist};
          fFiducialMatrix->LocalToMaster(newPtLocal, master);
          vertices.emplace_back(master[0], master[1], master[2]);
        }
      }
    }

    //TODO: Change "false" back to "true" to draw trajectories by default
    auto row = fViewer.AddDrawable<mygl::Path>("Trajectories", fNextID, parent, true, glm::mat4(), vertices, glm::vec4((glm::vec3)color, 1.0), fLineWidth); 
    row[fTrajRecord.fPartName] = traj.Name;
    auto p = traj.InitialMomentum;
    const double invariantMass = std::sqrt((p.Mag2()>0)?p.Mag2():0); //Make sure the invariant mass is > 0 as it should be.  It might be negative for 
                                                                       //photons because of floating point precision behavior.  Never trust a computer to 
                                                                       //calculate 0...
    row[fTrajRecord.fEnergy] = p.E()-invariantMass; //Kinetic energy
    Gdk::RGBA gdkColor;
    gdkColor.set_rgba(color.r, color.g, color.b, 1.0);
    row[fTrajRecord.fColor] = gdkColor;
    ++fNextID;

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
      auto ptRow = AddTrajPt(traj.Name, point, parentPt, glm::vec4(color, 1.0));
      const auto& subChildren = ptToTraj[ptIt];
      for(const auto& child: subChildren)
      {
        AppendTrajectory(row, child, parentToTraj, ptRow);
      }
    }
  }

  void EvdWindow::make_scenes()
  {
    //Configure Geometry Scenes
    for(const auto& drawPtr: fGlobalDrawers) drawPtr->RequestScenes(fViewer);    

    //Configure trajectory Scene
    auto& trajTree = fViewer.MakeScene("Trajectories", fTrajRecord, "/home/aolivier/app/evd/src/gl/shaders/colorPerVertex.frag", "/home/aolivier/app/evd/src/gl/shaders/colorPerVertex.vert", "/home/aolivier/app/evd/src/gl/shaders/wideLine.geom");
    trajTree.append_column("Particle Type", fTrajRecord.fPartName);
    //trajTree.insert_column_with_data_func(-1, "Particle", fPartNameRender, sigc::mem_fun(*this, &EvdWindow::ColToColor));
    trajTree.append_column("KE [MeV]", fTrajRecord.fEnergy);
    trajTree.insert_column_with_data_func(-1, "Color", fColorRender, sigc::mem_fun(*this, &EvdWindow::ColToColor));

    //Configure guide scene
    auto& guideTree = fViewer.MakeScene("Guides", fGuideRecord, "/home/aolivier/app/evd/src/gl/shaders/userColor.frag", "/home/aolivier/app/evd/src/gl/shaders/HUD.vert", "/home/aolivier/app/evd/src/gl/shaders/wideLine.geom"); 
    guideTree.append_column("Name", fGuideRecord.fName);
    guideTree.expand_to_path(Gtk::TreePath("0"));
    DrawGuides(); //Only do this once ever

    //Configure energy deposit Scene
    auto& edepTree = fViewer.MakeScene("EDep", fEDepRecord, "/home/aolivier/app/evd/src/gl/shaders/colorPerVertex.frag", "/home/aolivier/app/evd/src/gl/shaders/colorPerVertex.vert", "/home/aolivier/app/evd/src/gl/shaders/wideLine.geom");
    edepTree.append_column("Main Contributor", fEDepRecord.fPrimName);
    edepTree.append_column("Energy [MeV]", fEDepRecord.fEnergy);
    edepTree.append_column("dE/dx [MeV*cm^2/g]", fEDepRecord.fdEdx);
    edepTree.append_column("Scintillation Energy [MeV]", fEDepRecord.fScintE);
    edepTree.append_column("Start Time [ns?]", fEDepRecord.fT0);

    //Configure Trajectory Point Scene
    auto& ptTree = fViewer.MakeScene("TrajPts", fTrajPtRecord, "/home/aolivier/app/evd/src/gl/shaders/colorPerVertex.frag", "/home/aolivier/app/evd/src/gl/shaders/colorPerVertex.vert", "/home/aolivier/app/evd/src/gl/shaders/widePoint.geom");
    ptTree.append_column("Particle Type", fTrajPtRecord.fParticle);
    ptTree.append_column("Process", fTrajPtRecord.fProcess);
    ptTree.append_column("Momentum [MeV/c]", fTrajPtRecord.fMomMag);
    ptTree.append_column("Time", fTrajPtRecord.fTime);

    //TODO: Make fFileName a command line option to the application that runs this window.
    choose_file();
  }
  
  void EvdWindow::Print()
  {
    auto window = Gdk::Window::get_default_root_window(); //get_window() //TODO: Why does the main window crash this function?   
    const auto image = Gdk::Pixbuf::create(window, 0, 0, window->get_width(), window->get_height());
    const std::string type = "png"; //TODO: Let the user choose this
    std::stringstream name;
    auto strippedName = fFileName.substr(0, fFileName.find_first_of('.')); 
    name << "evd_" << strippedName.substr(strippedName.find_last_of("/")+1, std::string::npos) << "_event_" << fReader->GetCurrentEntry() << "." << type;
    image->save(name.str(), type);
  }

  void EvdWindow::build_toolbar()
  {
    fNavBar.add(fPrint);
    fPrint.signal_clicked().connect(sigc::mem_fun(*this, &EvdWindow::Print));
    
    fEvtNumWrap.add(fEvtNum);
    fNavBar.add(fEvtNumWrap);
    fEvtNum.signal_activate().connect(sigc::mem_fun(*this, &EvdWindow::goto_event));

    fNavBar.add(fNext);
    fNext.signal_clicked().connect(sigc::mem_fun(*this, &EvdWindow::next_event));

    fNavBar.add(fReload);
    fReload.signal_clicked().connect(sigc::mem_fun(*this, &EvdWindow::ReadEvent));
    
    fNavBar.add(fFileChoose);
    fFileChoose.signal_clicked().connect(sigc::mem_fun(*this, &EvdWindow::choose_file));
    fFileLabelWrap.add(fFileLabel);
    fNavBar.add(fFileLabelWrap);

    fFiducialLabelWrap.add(fFiducialLabel);
    fNavBar.add(fFiducialLabelWrap);
    fFiducialWrap.add(fFiducialName);
    fNavBar.add(fFiducialWrap);
    fFiducialName.signal_activate().connect(sigc::mem_fun(*this, &EvdWindow::set_fiducial));
  }

  void EvdWindow::choose_file()
  {
    Gtk::FileChooserDialog chooser(*this, "Choose an edep-sim file to view");
    //TODO: configure dialog to only show .root files and directories

    chooser.add_button("Open", Gtk::RESPONSE_OK);

    const int result = chooser.run(); 

    if(result == Gtk::RESPONSE_OK)
    {
      fFileName = chooser.get_filename();
      SetFile(fFileName); //TODO: Either remove fFileName or update SetFile() to use fFileName
      std::cout << "Set file to " << fFileName << "\n";
      fFileLabel.set_text(fFileName);
    }
  }

  void EvdWindow::next_event()
  {
    if(fReader->Next()) ReadEvent();
    else fReader->Restart(); //hopefully avoids disaster on next attempt to seek to an event
  }

  void EvdWindow::goto_event()
  {
    const auto newEvt = std::stoi(fEvtNum.get_text());
    if(fReader->SetEntry(newEvt) == TTreeReader::kEntryValid) 
    {
      ReadEvent();
    }
    else std::cerr << "Failed to get event " << newEvt << " from file " << fFileName << "\n";
  }

  void EvdWindow::ColToColor(Gtk::CellRenderer* render, const Gtk::TreeModel::iterator& it)
  {
    Gdk::RGBA color = (*it)[fTrajRecord.fColor];
    auto textRender = ((Gtk::CellRendererText*)render);
    textRender->property_background_rgba().set_value(color);
  }

  void EvdWindow::set_fiducial()
  {
    const std::string nodeName = fFiducialName.get_text();

    auto id = new TGeoIdentity();
    if(find_node(nodeName, fGeoManager->GetTopNode(), *id))
    {
      std::cout << "Set fiducial node to " << fFiducialNode->GetName() << "\n";
      ReadEvent();
    }
    else
    {
      fFiducialName.set_text(fFiducialNode->GetName());
      std::cerr << "Failed to find node with name " << nodeName << " in file " << fFile->GetName() << "\n";
    }
  }

  bool EvdWindow::find_node(const std::string& name, TGeoNode* parent, TGeoMatrix& mat)
  {
    TGeoHMatrix local(mat);
    local.Multiply(parent->GetMatrix());
    if(std::string(parent->GetName()) == name) 
    {
      fFiducialNode = parent;
      fFiducialMatrix = new TGeoHMatrix(local);
      return true;
    }

    auto children = parent->GetNodes();
    for(auto child: *children)
    {
      auto node = (TGeoNode*)child;
      if(find_node(name, node, local)) return true;
    }
    return false;
  }
}


