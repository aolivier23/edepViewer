//File: DefaultGeo.h
//Brief: A plugin that draws the ROOT geometry for the edepsim display.  Takes a TGeoManager as drawing data and 
//       draws 3D shapes using ROOT's tesselation facilities.  
//Author: Andrew Olivier aolivier@ur.rochester.edu

//gl includes
#include "gl/SceneConfig.cpp"

//draw includes
#include "plugins/drawing/GeoDrawer.cpp"

//ROOT includes
#include "TGeoManager.h" //For data

//yaml-cpp include for configuration
#include "yaml-cpp/yaml.h"

#ifndef DRAW_DEFAULTGEO_H
#define DRAW_DEFAULTGEO_H

namespace mygl
{
  class ColorIter;
  class ColRecord;
}

namespace draw
{
  class DefaultGeo: public GeoDrawer
  {
    public:
      DefaultGeo(const YAML::Node& config);
      virtual ~DefaultGeo() = default;

      virtual void RemoveAll(mygl::Viewer& viewer) override;

    protected:
      virtual void doRequestScenes(mygl::Viewer& viewer) override;
      virtual void doDrawEvent(const TGeoManager& data, mygl::Viewer& viewer, mygl::VisID& nextID) override;

      //Helper functions for drawing geometry volumes
      virtual void AppendNode(mygl::Scene& scene, mygl::VisID& nextID, TGeoNode* node, glm::mat4& mat, 
                              const mygl::TreeModel::iterator parent, size_t depth);
      virtual void AppendChildren(mygl::Scene& scene, mygl::VisID& nextID, const mygl::TreeModel::iterator parent, 
                                  TGeoNode* parentNode, glm::mat4& mat, size_t depth);

      //Data needed when appending geometry nodes
      size_t fMaxDepth; //The maximum depth drawn in geometry hierarchy
      std::unique_ptr<mygl::ColorIter> fColor;

      //ColRecord-derived classes to make unique TreeViews for geometry and trajectories
      class GeoRecord: public mygl::ColRecord
      {
        public:
          GeoRecord(): ColRecord(), fName("Name"), fMaterial("Material")
          {
            Add(fName);
            Add(fMaterial);
          }

          mygl::TreeModel::Column<std::string> fName;
          mygl::TreeModel::Column<std::string> fMaterial;
      };

      std::shared_ptr<GeoRecord> fGeoRecord;

      class GeoConfig: public mygl::SceneConfig
      {
        public:
          GeoConfig() = default;
          virtual ~GeoConfig() = default;

          virtual void BeforeRender() override
          {
          }

          virtual void AfterRender() override
          {
          }
      };
  };
}

#endif //DRAW_DEFAULTGEO_H
