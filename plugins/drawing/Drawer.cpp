//File: Drawer.cpp
//Brief: A Drawer dispatches its base class' Draw() calls to a thread so that rendering can 
//       happen in the "main" thread while future events are being processed.  This is an 
//       implementation detail that centralizes how function calls are dispatched, so users should 
//       never interact with it directly.  
//Author: Andrew Olivier aolivier@ur.rochester.edu

//yaml-cpp includes
#include "yaml-cpp/yaml.h"

//draw includes
#include "plugins/drawing/Services.cpp"

//gl includes
#include "gl/Viewer.h"
#include "gl/model/Drawable.h"
#include "gl/scene/SceneModel.cpp"

//c++ includes
#include <future>

#ifndef DRAW_DETAIL_DRAWER_CPP
#define DRAW_DETAIL_DRAWER_CPP

class TGeoManager;
class TG4Event;

namespace draw
{
  namespace detail
  {
    template <class ...ARGS>
    class DrawerBase
    {
      public:
        DrawerBase(const YAML::Node& config): fDefaultDraw(false)
        {
          if(config["DefaultDraw"]) fDefaultDraw = config["DefaultDraw"].as<bool>();
        }
                                                                                     
        virtual ~DrawerBase() = default;
                                                                                     
        //Request the needed Scene(s) from the Viewer
        virtual void RequestScene(mygl::Viewer& viewer) = 0;
                                                                                     
        //Map data from TGeoManager to Drawables associated with metadata
        virtual void Draw(ARGS... args) = 0;
                                                                                     
        //Try to update SceneController for the next event.  If model_t for the 
        //next event is not yet ready, return false.  Otherwise, return true.  
        virtual bool TryUpdateScene() = 0;
                                                                                     
        //Clear the current queue of events because random access has happened
        virtual void Clear() = 0;
                                                                                     
      protected:
        //Configuration that applies to all GeoDrawers
        bool fDefaultDraw; //Should this Drawer's elements be drawn by default?
    };

    template <class HANDLE, class ...ARGS>
    class Drawer: public DrawerBase<ARGS...>
    {
      protected:
        using model_t = ctrl::SceneModel<HANDLE>;
        using scene_t = ctrl::SceneController; //TODO: SceneController<HANDLE>
    
        Drawer(const YAML::Node& config): DrawerBase<ARGS...>(config), fScene(nullptr)
        {
        }

        virtual ~Drawer() = default; //fScene is not owned.  

        //Provide a public interface, but call protected interface functions so that I can add 
        //behavior common to all GeoDrawers here.  The user should implement these functions only. 
        //TODO: The return value of doRequestScene() should probably change when SceneController becomes a base class
        virtual scene_t& doRequestScene(mygl::Viewer& viewer) = 0; //Creates a Scene and returns its name
        virtual std::unique_ptr<model_t> doDraw(ARGS... args) = 0; //Processes a geometry model and returns 
                                                                   //a model_t to draw it 

        virtual void RequestScene(mygl::Viewer& viewer) override final
        {
          fScene = &(doRequestScene(viewer)); //Take an observer pointer that is only valid as long as viewer exists
          assert(fScene != nullptr);
        }

        //Do geometry processing in a new thread
        virtual void Draw(ARGS... args) override final
        {
          fModelInProcessing = std::async(std::launch::async, doDraw, args...); 
        }

        //For now, try to give fModelInProcessing to fScene.  Return true if 
        //successful and false if fModelInProcessing is not yet ready.  
        //TODO: Try to get the next model off of fModelCache.  If fModelCache is empty and fModelInProcessing is not ready, 
        //      return false.  Otherwise, transfer fModelInProcessing directly to fScene and return true.  
        virtual bool TryUpdateScene() override final
        {
          /*assert(!fModelCache.empty());
          const auto newModel = std::move(fModelCache.front());
          fModelCache.pop();*/
          if(fModelInProcessing.wait_for(std::chrono::milliseconds(10)) == std::future_status::timeout) return false;

          assert(fScene != nullptr);
          fScene->NewEvent(std::move(fModelInProcessing));
          return true;
        }

        virtual void Clear() override final
        {
          //fModelCache = std::queue<model_t>();
        }

      private:
        scene_t* fScene; //Observer pointer to the SceneController that this plugin configures
                                               //TODO: The type of fScene
        //std::queue<std::unique_ptr<model_t>> fModelCache; //TODO: Cache of data to draw for upcoming events
        std::future<std::unique_ptr<model_t>> fModelInProcessing; //Model for the next geometry as its being drawn
    };
  }
}

#endif //DRAW_DETAIL_DRAWER_CPP 
