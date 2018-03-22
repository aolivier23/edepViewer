//File: Scene.h
//Brief: A Scene manages a collection of Drawables.  It is a view component, 
//       so it does not do anything.  It provides an interface that an event display 
//       controller component (currently considering a GLViewer class) can use to 
//       transfer information to the visible view.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//model includes
#include "gl/model/GenException.h"
#include "gl/model/ShaderProg.h"
#include "gl/VisID.h"
#include "gl/ColRecord.cpp"
#include "gl/UserCut.h"

//local includes
#include "gl/TreeModel.h"

//glm includes
#include <glm/glm.hpp>

//c++ includes
#include <map>
#include <limits>
#include <memory>
#include <iostream>

#ifndef VIEW_SCENE_H
#define VIEW_SCENE_H

namespace mygl
{
  class ShaderProg;
  class Drawable;

  class Scene
  {
    public:
      Scene(const std::string& name, const std::string& fragSrc, const std::string& vertSrc, std::shared_ptr<mygl::ColRecord>& cols);
      Scene(const std::string& name, const std::string& fragSrc, const std::string& vertSrc, const std::string& geomSrc, 
            std::shared_ptr<mygl::ColRecord>& cols);
      
      virtual ~Scene();

      //Warning: The following function may not do anything if Gtk::GLArea::make_current() is 
      //         not called just before.  
      virtual TreeModel::iterator AddDrawable(std::unique_ptr<Drawable>&& drawable, const VisID& id, const TreeModel::iterator parent, 
                                              const bool active);
      virtual TreeModel::iterator NewTopLevelNode();
      virtual void RemoveAll();

      //Draws all of the objects in fDrawables using fShader, view matrix, and persp(ective) matrix
      virtual void Render(const glm::mat4& view, const glm::mat4& persp);
      virtual void RenderGUI();
      virtual void RenderSelection(const glm::mat4& view, const glm::mat4& persp);

      const std::string fName; //The name of this scene.  Might be useful for list tree displays
      
      //GUI components that the Viewer will arrange
      //TODO: Restore GUI components using ImGUI 
      //Gtk::TreeView fTreeView; //The user is responsible for telling this TreeView about columns in cols other than the 
                                 //standard columns in evd::ColRecord.
      //mygl::UserCut fCutBar; //Allows the user to perform cuts on visible data

      //void draw_self(const Glib::ustring& path); //TODO: Restore this functionality
      //void start_filtering();
      bool filter(const TreeModel::iterator& iter);  //I need to wrap over UserCut's function to disable rows that are hidden
      void remove_row(const TreeModel::iterator& path);

      bool SelectID(const mygl::VisID& id);
      void on_tree_selection();

    protected:
      //opengl components
      std::map<VisID, std::unique_ptr<Drawable>> fActive; //The opengl drawing instructions for this scene that will be drawn on Render
      std::map<VisID, std::unique_ptr<Drawable>> fHidden; //Opengl drawing instructions for this scene that will not be drawn on Render
      ShaderProg fShader; //The opengl shader program with which the objects in fDrawables will be rendered
      ShaderProg fSelectionShader; //Same components as fShader except for the fragment shader.  This program 
                                   //uses a special (unique?) fragment shader to which it can bind a VisID as 
                                   //a color.

      //Metadata model
      TreeModel fModel;
      std::shared_ptr<mygl::TreeModel::ColumnModel> fCols;

      //Data needed for highlighting
      mygl::VisID fSelection; //The currently selected VisID

      //Helper function for drawing tree
      void DrawNode(const mygl::TreeModel::iterator iter);

    private: 
      void Transfer(std::map<VisID, std::unique_ptr<Drawable>>& from, std::map<VisID, std::unique_ptr<Drawable>>& to, const VisID& id);      

      //Details to save user code when working with the TreeModel
      TreeModel::Column<bool> fSelfCol;
      TreeModel::Column<VisID> fIDCol;

      void BuildGUI(std::shared_ptr<mygl::ColRecord>& cols);
  };
}

#endif //VIEW_SCENE_H
