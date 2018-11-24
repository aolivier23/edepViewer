//File: Controller.cpp
//Brief: A Controller dispatches its base class' Draw() calls to a thread so that rendering can 
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
#include <queue>

#ifndef DRAW_DETAIL_DRAWER_CPP
#define DRAW_DETAIL_DRAWER_CPP

class TGeoManager;
class TG4Event;

namespace draw
{
  namespace detail
  {
    //Interface exposed to the application.  I'll give the user lots of help 
    //implementing it below with a derived class template.  I don't really 
    //expect anyone to ever derive directly from ControllerBase.
    template <class ...ARGS>
    class ControllerBase
    {
      public:
        ControllerBase(const YAML::Node& config)
        {
        }
                                                                                     
        virtual ~ControllerBase() = default;
                                                                                     
        //Request the needed Scene(s) from the Viewer
        virtual void RequestScene(mygl::Viewer& viewer) = 0;
                                                                                     
        //Map data from TGeoManager to Drawables associated with metadata
        virtual void Draw(ARGS... args) = 0;
                    
        //Try to update SceneController for the next event.  If model_t for the 
        //next event is not yet ready, return false.  Otherwise, return true.  
        virtual void UpdateScene(mygl::VisID& nextID) = 0;
                                                                                     
        //Clear the current queue of events because random access has happened
        virtual void Clear() = 0;
    };

    //A Controller implements ControllBase's interface by knowing the type of a DRAWER it 
    //owns.  DRAWER shall:
    //1) Be constructible from a const YAML::Node
    //2) Provide the signature: std::unique_ptr<model_t> doDraw(ARGS...) 
    //3) Provide the signature: scene_t& doRequestScene(mygl::Viewer&)
    template <class DRAWER, class ...ARGS>
    class Controller: public ControllerBase<ARGS...>
    {
      public:
        using scene_t = typename std::remove_reference<decltype(std::declval<DRAWER>().doRequestScene(std::declval<std::add_lvalue_reference<mygl::Viewer>::type>()))>::type;
        using model_t = typename scene_t::model_t; //typename decltype(std::declval<DRAWER>().doDraw(std::declval<ARGS>()...))::element_type; 
    
        Controller(const YAML::Node& config): ControllerBase<ARGS...>(config), fScene(nullptr), fDrawer(config)
        {
        }

        virtual ~Controller() = default; //fScene is not owned.  

        virtual void RequestScene(mygl::Viewer& viewer) override final
        {
          fScene = &(fDrawer.doRequestScene(viewer)); //Take an observer pointer that is only valid as long as viewer exists
          assert(fScene != nullptr);
        }

        //Do geometry processing in a new thread
        virtual void Draw(ARGS... args) override final
        {
          //TODO: The model_t returned must be created in the rendering thread 
          //      because it creates buffers!
          fModelCache.push(fDrawer.doDraw(args...));
        }

        //Give the latest model_t created to fModelCache.  For now, each "class" of 
        //Controller shall be run in series, so this function shall be called only 
        //after a thread calling Draw() has finished.  
        virtual void UpdateScene(mygl::VisID& nextID) override final
        {
          assert(!fModelCache.empty());
          auto newModel = std::move(fModelCache.front());
          fModelCache.pop();
          //TODO: Cache the previous event?

          assert(fScene != nullptr);
          fScene->NewEvent(std::move(newModel), nextID);
        }

        virtual void Clear() override final
        {
          fModelCache = std::queue<std::unique_ptr<model_t>>();
        }

      private:
        scene_t* fScene; //Observer pointer to the SceneController that this Controller associates with its Drawer. 
                         //Holding a pointer is necessary to delay initialization.  
        DRAWER fDrawer; //Algorithm for filling a scene_t given an event
        std::queue<std::unique_ptr<model_t>> fModelCache; //Cached of data to draw for upcoming events
                                                          //TODO: Cache the previous event as well?
    };
  }
}

//typedefs to ease legacy plugin support
namespace legacy
{
  using scene_t = ctrl::SceneController; //<mygl::Drawable>;
  using model_t = ctrl::SceneModel<mygl::Drawable>;
}

#endif //DRAW_DETAIL_DRAWER_CPP 
