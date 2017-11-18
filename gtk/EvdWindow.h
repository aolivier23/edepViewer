//File: EvdWindow.h
//Brief: A Gtk::Window for displaying the geometry in a ROOT file.  Has the 
//       ability to draw individual volumes.  Draws both a list tree view of 
//       the geometry hierarchy and a 3D view. 
//Author: Andrew Olivier aolivier@ur.rochester.edu

//Gtkmm includes
#include <gtkmm.h>

//gl includes
#include "gl/Viewer.h"
#include "gl/Scene.h"
#include "gl/ColRecord.cpp"

//gtk directory includes
#include "ColorIter.cxx"
#include "gtk/Palette.cpp"

//ROOT includes
#include "TFile.h"
#include "TTreeReader.h"
#include "TGeoMatrix.h"
#include "TDatabasePDG.h"

//TODO: move EDepSim dependence out of this file
//EDepSim includes
#include "TG4Event.h" //TODO: Make CMake figure out this path

#ifndef EVD_EVDWINDOW
#define EVD_EVDWINDOW

//Forward declaration of ROOT classes
class TGeoNode;
class TGeoManager;

namespace mygl
{
  class EvdWindow: public Gtk::Window //Gtk::ApplicationWindow 
  {
    public: 
      EvdWindow(const std::string& fileName, const bool darkColors = true); //, const Glib::RefPtr<Gtk::Application>& app);
      virtual ~EvdWindow();

      void SetFile(const std::string& fileName); //Set the file to be processed
      //TODO: Overhaul drawing model to use boolean widgets in TreeView
      void Print(); //Print the current window to a file

      virtual void make_scenes();

    protected:
      //Child Widgets
      Glib::RefPtr<Gtk::TreeSelection> fSelection;
      mygl::Viewer fViewer;

      //Toolbar for event navigation and printing
      //TODO: event navigation
      Gtk::Box fVBox; //Vertical Box widget for positioning tool bar above drawing area
      Gtk::Toolbar fNavBar;
      Gtk::ToolButton fPrint; 
      Gtk::ToolButton fNext;
      Gtk::ToolButton fReload;
      Gtk::ToolItem fEvtNumWrap; 
      Gtk::Entry fEvtNum; 
      Gtk::ToolButton fFileChoose; 
      Gtk::ToolItem fFileLabelWrap;
      Gtk::Label fFileLabel; //Displays the current file name so that it shows up in printouts 

      //Source of data for drawing
      std::string fFileName;
      std::unique_ptr<TFile> fFile;
      std::unique_ptr<TTreeReader> fReader;
      std::unique_ptr<TTreeReaderValue<TG4Event>> fCurrentEvt; //The current event being drawn.
      TGeoManager* fGeoManager; //The current geometry 

    private:
      Gtk::TreeModel::Row AppendNode(TGeoNode* node, TGeoMatrix& mat, const Gtk::TreeModel::Row& parent, size_t depth);
      void AppendChildren(const Gtk::TreeModel::Row& parent, TGeoNode* parentNode, TGeoMatrix& mat, size_t depth);
      void AppendTrajectories(const Gtk::TreeModel::Row& parent, const int id, std::map<int, std::vector<TG4Trajectory>>& parentToTraj);
      void ReadGeo();
      void ReadEvent();
      void DrawGuides(); 

      mygl::VisID fNextID;
      std::unique_ptr<mygl::ColorIter> fGeoColor; //unique_ptr so I can reset() it
      mygl::ColorIter fPDGColor;
      const size_t fMaxGeoDepth; //The maximum recursion depth when processing a geometry file
      VisID fAfterLastGeo; //The VisID after the last VisID used for the geometry.  Allows reusing VisIDs for event objects 

      std::map<int, glm::vec3> fPDGToColor; //TODO: A separate interface from the main window for better organization.  
      TDatabasePDG fPdgDB;

      void build_toolbar(); //TODO: Write a custom Toolbar class that does this buidling
      void choose_file(); 
      void goto_event();
      void next_event();

      //Information for fiducial volume cut
      Gtk::ToolItem fFiducialLabelWrap;
      Gtk::Label fFiducialLabel;
      Gtk::ToolItem fFiducialWrap;
      Gtk::Entry fFiducialName;
      TGeoNode* fFiducialNode; //The node for the fiducial volume.  Trajectory points outside this volume will not be drawn.
      TGeoMatrix* fFiducialMatrix; //A matrix that translates objects from the top Node's coordinate system to fFiducialNode's 
                                   //coordinate system. 

      void set_fiducial();
      bool find_node(const std::string& name, TGeoNode* parent, TGeoMatrix& mat);

      //Drawing settings
      float fLineWidth; //The default line width to use

      //ColRecord-derived classes to make unique TreeViews for geometry and trajectories
      class GeoRecord: public ColRecord
      {
        public:
          GeoRecord(): ColRecord()
          {
            add(fName);
            add(fMaterial);
          }
  
          Gtk::TreeModelColumn<std::string> fName;
          Gtk::TreeModelColumn<std::string> fMaterial;
      };

      GeoRecord fGeoRecord;

      class TrajRecord: public ColRecord
      {
        public:
          TrajRecord(): ColRecord()
          {
            add(fPartName);
            add(fEnergy);
            add(fColor);
            //add(fProcess); //TODO: Learn to get end process from Track
          }
       
          Gtk::TreeModelColumn<std::string> fPartName; 
          Gtk::TreeModelColumn<double> fEnergy;
          Gtk::TreeModelColumn<Gdk::RGBA> fColor;
          //Gtk::TreeModelColumn<std::string> fProcess;
      };

      TrajRecord fTrajRecord;
      Gtk::CellRendererText fColorRender; //Customized renderer for particle name to write name in color
      void ColToColor(Gtk::CellRenderer* render, const Gtk::TreeModel::iterator& it);

     class GuideRecord: public ColRecord
     {
       public:
         GuideRecord(): ColRecord()
         {
           add(fName);
         }

         Gtk::TreeModelColumn<std::string> fName;
     };

     GuideRecord fGuideRecord;

     class EDepRecord: public ColRecord
     {
       public: 
         EDepRecord(): ColRecord()
         {
           add(fPrimName);
           add(fEnergy);
           add(fdEdx);
           add(fT0);
           add(fScintE);
           //TODO: Is it fair to call energy/length dE/dx?  
         }

         Gtk::TreeModelColumn<double>      fEnergy;
         Gtk::TreeModelColumn<std::string> fPrimName;
         Gtk::TreeModelColumn<double>      fT0;
         Gtk::TreeModelColumn<double>      fScintE;
         Gtk::TreeModelColumn<double>      fdEdx;
     };
   
     EDepRecord fEDepRecord;

     //TODO: If this is useful, consider moving it to its' own file
     /*class Palette
     {
       public:
         Palette(const double min, const double max): fMin(min), fMax(max) {}
         virtual ~Palette() = default;

         glm::vec3 operator ()(const double value) //Returns a color between red and blue.  Red corresponds to high energy, and blue corresponds to low 
                                                   //energy.
         {
           if(value > fMax) return glm::vec3(0.0, 1.0, 0.0);
           if(value < fMin) return glm::vec3(0.0, 0.0, 1.0);

           const double red = (value - fMin)/(fMax-fMin);
           if(red < 0.5) return glm::vec3(0., 2.*red, 1.0-2.*red);
           else return glm::vec3(2.*red-1.0, 2.*red-1.0, 0.); 
           //return glm::vec3(red, std::fabs(0.5-red), 1.0-red);
         }

       protected:
         const double fMax;
         const double fMin;
     };*/

     Palette fPalette;
  };
}

#endif //EVD_EVDWINDOW  
