//File: DefaultGeo.h
//Brief: A plugin that draws the ROOT geometry for the edepsim display.  Takes a TGeoManager as drawing data and 
//       draws 3D shapes using ROOT's tesselation facilities.  
//Author: Andrew Olivier aolivier@ur.rochester.edu

//gl includes
#include "gl/scene/SceneConfig.cpp"

//draw includes
#include "GeoController.cpp"

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
  class DefaultGeo
  {
    public:
      DefaultGeo(const YAML::Node& config);
      virtual ~DefaultGeo() = default;

      virtual legacy::scene_t& doRequestScene(mygl::Viewer& viewer);
      virtual std::unique_ptr<legacy::model_t> doDraw(const TGeoManager& data, Services& /*services*/);

    private:
      //Helper functions for drawing geometry volumes
      virtual void AppendNode(legacy::model_t::view& parent, TGeoNode* node, glm::mat4& mat, size_t depth);
      virtual void AppendChildren(legacy::model_t::view& parent, TGeoNode* parentNode, glm::mat4& mat, size_t depth);

      //Data needed when appending geometry nodes
      size_t fMaxDepth; //The maximum depth drawn in geometry hierarchy
      bool fDefaultDraw; //Whether to draw all geometry nodes by default
      std::unique_ptr<mygl::ColorIter> fColor; //TODO: Use some Service here instead?

      //ColRecord-derived classes to make unique TreeViews for geometry and trajectories
      class GeoRecord: public ctrl::ColumnModel
      {
        public:
          GeoRecord(): ColumnModel(), fName("Name"), fMaterial("Material")
          {
            Add(fName);
            Add(fMaterial);
          }

          ctrl::Column<std::string> fName;
          ctrl::Column<std::string> fMaterial;
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
