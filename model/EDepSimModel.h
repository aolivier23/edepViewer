//File: EDepSimModel.h
//Brief: A Model that reads truth information from a ROOT file produced by the 
//       EDepSim framework.  Produces Drawables for true trajectories and energy deposits, 
//       and reads in geometry from the EDepSim file's TGeoManager.  The EDepSimController 
//       should manage access to the TTree being read via a TTreeReader
//Author: Andrew Olivier aolivier@ur.rochester.edu

//model includes
#include "model/Model.h"

//ROOT includes with class templates (they exist!)
#include "TTreeReader.h"

#ifndef MODEL_EDEPSIMMODEL_H
#define MODEL_EDEPSIMMODEL_H

namespace model
{
  class EDepSimModel: public Model
  {
    public:
      EDepSimModel(std::shared_ptr<TFile> file, TTreeReader& reader);
      virtual EDepSimModel() = default;

      virtual std::vector<std::pair<std::unique_ptr<view::Drawable>, view::VisID>> ProduceEvent();
      virtual std::vector<std::pair<std::unique_ptr<view::Drawable>, view::VisID>> ProduceGeo();
      virtual Glib::RefPtr<Gtk::TreeStore> ProduceGeoTree();
      virtual Glib::RefPtr<Gtk::TreeStore> ProduceEvtTree();

    protected:
      std::shared_ptr<TFile> fFile; //Hold a shared_ptr to the file being read so that it cannot be destroyed 
                                    //while I am still reading it
      TGeoManager* fGeoMan; //observer pointer
      TTreeReaderValue<> fTrajectories; //TODO: type of trajectories
      TTreeReaderValue<> fEvent; //TODO: type of Event from EDepSim
  };
}

#endif //MODEL_EDEPSIMMODEL_H
