//File: SceneController.h
//Brief: A SceneController can Draw() a SceneControllerModel using OpenGL and display its contents as a GUI.  
//Author: Andrew Olivier aolivier@ur.rochester.edu

//TODO:
//Proposed changes: I want to be able to generate SceneControllers asyncrhonously and buffer SceneControllers 
//                  for events that I think the user might want to view next.  So, SceneControllers 
//                  should probably become copyable.  I realized that I don't ever expose 
//                  a Drawable through SceneController, which has unique ownership of all Drawables. 
//                  So, I could probably write an abstract SceneController base class and a derived 
//                  class template to loosen the requirements on drawn GL objects from a 
//                  c++ inheritance relationship to a contract/concept.    
//
//                  Currently, a SceneController binds together through composition a couple of models 
//                  that work together to present rendered 3D and GUI objects to the user. 
//                  First, it owns a mapping of VisIDs to metadata rows as a TreeModel.  
//                  The loose coupling between this TreeModel and the fActive and fHidden 
//                  Drawables seems a little clunky and bug-prone, but I'm tempted not to 
//                  interfere with it as this stage as long as it works.  I should probably 
//                  look out for unique_ptrs in TreeModel though.  Next, a SceneController owns Drawables
//                  as non-copyable unique_ptrs in a pair of maps.  With the strategy in the 
//                  previous paragraph, I think I could remove the need for unique_ptrs entirely 
//                  if that would help.  Finally, a SceneController owns a VAO, an OpenGL vertex buffer,
//                  that is coupled tightly to the Drawables it owns.  Each Drawable has a 
//                  "reference" to the data it needs from its SceneController's VAO.  Furthermore, the 
//                  VAO's Use() member function buffers data on the graphics card, so it 
//                  cannot be called from another thread.  
//
//                  Right now, SceneController itself has a few member elements that are used throughout 
//                  its lifetime: name, column model, ShaderProg, SceneConfig, and maybe TreeModel
//                  depending on how it's implemented.  Seems like I just want to buffer "TreeModel 
//                  structure", VAO, and Drawable-VisID mapping, so thinking about moving those 
//                  components into their own object.  Since "the user" will create this object and 
//                  SceneController will take sole ownership of it, I'm thinking I should store a unique_ptr 
//                  to it that can be overridden each time a new event becomes the "current" event. 
//                  It seemes like the PIMPL idiom is also "a generally good thing to do".  
// 
//                  What object will own the queue of "SceneControllerModels"?  The EvdWindow needs to 
//                  trigger the move to the next SceneControllerModel and react to it missing by waiting on 
//                  a thread.  Realized that I should probably just assign VisIDs when a SceneControllerModel 
//                  becomes the current event so that I can make better use of the volume of available 
//                  VisIDs, and I could take this opportunity to remove them from the algorithm 
//                  interface altogether.  I think that keeping track of VisIDs might also remove 
//                  the remaining concurrency problem that is stopping me from running each algorithm 
//                  in its own thread. To delay the assignment of VisIDs, I think that I have to find 
//                  a different method of coupling metadata rows to Drawables (or whatever template 
//                  parameter they become).  
//
//                  Reliance on SceneController::Transfer() seems like obstacle for dealing with metadata-Drawable 
//                  coupling without VisID.  It is used in DrawNode() and ApplyFilter(). Also, Viewer::on_selection() 
//                  uses SceneController::SelectID() which is the real reason why I started using VisIDs to begin 
//                  with.  Seems like fSelectPath might also be an obstacle.  Seems to me like pointer-based 
//                  coupling between Drawable and metadata row might solve 3/4 problems. The Viewer::on_selection() 
//                  interface needs to work like it does now.   
//
//                  A map from VisID to (Drawable pointer, TreeModel::iterator) might be a good way to implement 
//                  SceneController::SelectID().  Seems like I designed around embedding a reference to Drawable into 
//                  each TreeModel row.   
//
//TODO: SceneController is also acting as a TreeModel view right now.  Should I separate that behavior into its own class?  

//model includes
#include "util/GenException.h"
#include "gl/selection/VisID.h"
#include "gl/selection/UserCut.h"
#include "gl/objects/ShaderProg.h"
#include "gl/objects/VAO.h"

//glm includes
#include <glm/glm.hpp>

//c++ includes
#include <limits>
#include <memory>
#include <vector>

#ifndef VIEW_SCENE_H
#define VIEW_SCENE_H

namespace mygl
{
  class Drawable;
  class SceneConfig;
  class VAO;
}

namespace ctrl
{
  template <class HANDLE>
  class SceneModel;

  class ColumnModel;

  template <class HANDLE>
  class TreeNode;

  class SceneController
  {
    public:
      using handle_t = mygl::Drawable;
      using model_t = SceneModel<handle_t>;
      using node_t = TreeNode<std::unique_ptr<handle_t>>;

      SceneController(const std::string& fragSrc, const std::string& vertSrc, std::shared_ptr<ColumnModel>& cols, 
            std::unique_ptr<mygl::SceneConfig>&& config);
      SceneController(const std::string& fragSrc, const std::string& vertSrc, const std::string& geomSrc, 
            std::shared_ptr<ColumnModel>& cols, std::unique_ptr<mygl::SceneConfig>&& config);
      
      virtual ~SceneController();

      //Load objects to draw for a new event and get rid of old event's model.  This is the only way that the user 
      //interacts with SceneController now.  
      void NewEvent(std::unique_ptr<model_t>&& newModel, mygl::VisID& nextID);

      //Functions for drawing objects associated with this Scene
      virtual void Render(const glm::mat4& view, const glm::mat4& persp);
      virtual void RenderGUI();
      virtual void RenderSelection(const glm::mat4& view, const glm::mat4& persp);

      //Function for applying object selection
      bool SelectID(const mygl::VisID& id);

    protected:
      //Helper functions for drawing tree
      bool DrawNodeData(node_t& node);

    private: 
      //Data for GUI operations
      std::vector<mygl::VisID> fSelectPath; //The path to the currently-selected Node
      mygl::UserCut fCutBar; //Allows the user to perform cuts on visible data

      //Data for working with current ColumnModel
      std::shared_ptr<ColumnModel> fCols; //Columns displayed by this Scene

      //OpenGL rendering data used for all events
      std::unique_ptr<mygl::SceneConfig> fConfig; //User handle to configure openGL just before and after rendering. 
      mygl::ShaderProg fShader; //The opengl shader program with which the objects in fDrawables will be rendered
      mygl::ShaderProg fSelectionShader; //Same components as fShader except for the fragment shader.  This program 
                                   //uses a special (unique?) fragment shader to which it can bind a VisID as 
                                   //a color.
      mygl::VAO fVAO; //A place to store vertices on the GPU 

      //Data specific to the current event
      std::unique_ptr<model_t> fCurrentModel; //The SceneModel that is currently being drawn
  };
}

#endif //VIEW_SCENE_H
