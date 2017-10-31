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

//ROOT includes
#include "TFile.h"
#include "TTreeReader.h"
#include "TGeoMatrix.h"
#include "TDatabasePDG.h"

//TODO: move EDepSim dependence out of this file
//EDepSim includes
#include "/home/aolivier/app/edep-sim/src/TG4Event.h" //TODO: Make CMake figure out this path

#ifndef EVD_EVDWINDOW
#define EVD_EVDWINDOW

//Forward declaration of ROOT classes
class TGeoNode;

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
      Gtk::ToolItem fEvtNumWrap; 
      Gtk::Entry fEvtNum; 
      Gtk::ToolButton fFileChoose;  

      //Source of data for drawing
      std::string fFileName;
      std::unique_ptr<TFile> fFile;
      std::unique_ptr<TTreeReader> fReader;
      std::unique_ptr<TTreeReaderValue<TG4Event>> fCurrentEvt; //The current event being drawn.

    private:
      Gtk::TreeModel::Row AppendNode(TGeoNode* node, TGeoMatrix& mat, const Gtk::TreeModel::Row& parent, size_t depth);
      void AppendChildren(const Gtk::TreeModel::Row& parent, TGeoNode* parentNode, TGeoMatrix& mat, size_t depth);
      void AppendTrajectories(const Gtk::TreeModel::Row& parent, const int id, std::map<int, std::vector<TG4Trajectory>>& parentToTraj);
      void ReadGeo();
      void ReadEvent();
      //void DrawGuides(); //TODO: Move this to Viewer

      mygl::VisID fNextID;
      mygl::ColorIter fGeoColor;
      mygl::ColorIter fPDGColor;
      const size_t fMaxGeoDepth; //The maximum recursion depth when processing a geometry file
      VisID fAfterLastGeo; //The VisID after the last VisID used for the geometry.  Allows reusing VisIDs for event objects 

      std::map<int, glm::vec3> fPDGToColor; //TODO: A separate interface from the main window for better organization.  
      TDatabasePDG fPdgDB;

      void build_toolbar(); //TODO: Write a custom Toolbar class that does this buidling
      void choose_file(); 
      void goto_event();
      void next_event();

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

     /*class GuideRecord: public ColRecord
     {
       public:
         GuideRecord(): ColRecord()
         {
           add(fName);
         }

         Gtk::TreeModelColumn<std::string> fName;
     };

     GuideRecord fGuideRecord;*/

     class EDepRecord: public ColRecord
     {
       public: 
         EDepRecord(): ColRecord()
         {
           add(fPrimName);
           add(fEnergy);
           add(fT0);
           add(fScintE);
           //TODO: Is it fair to call energy/length dE/dx?  
         }

         Gtk::TreeModelColumn<double>      fEnergy;
         Gtk::TreeModelColumn<std::string> fPrimName;
         Gtk::TreeModelColumn<double>      fT0;
         Gtk::TreeModelColumn<double>      fScintE;
     };
   
     EDepRecord fEDepRecord;

     //TODO: If this is useful, consider moving it to its' own file
     class RedToBluePalette
     {
       public:
         RedToBluePalette(const double min, const double max): fMin(min), fMax(max) {}
         virtual ~RedToBluePalette() = default;

         glm::vec3 operator ()(const double value) //Returns a color between red and blue.  Red corresponds to high energy, and blue corresponds to low 
                                                   //energy.
         {
           if(value > fMax) return glm::vec3(1.0, 0.0, 0.0);
           if(value < fMin) return glm::vec3(0.0, 0.0, 1.0);

           const double red = (value - fMin)/(fMax-fMin);
           return glm::vec3(red, 0, 1.0-red);
         }

       protected:
         const double fMax;
         const double fMin;
     };

     RedToBluePalette fPalette;
  };
}

#endif //EVD_EVDWINDOW  
