//File: SceneModel.cpp
//Brief: A SceneModel groups all of the resources needed to display a particular 
//       event in a Scene.  SceneModel can work with any HANDLE class including 
//       the old Drawable interface.  
//
//Implementation note: A SceneModel is not a TreeNode only because it does not own a 
//                     HANDLE itself.  SceneModel controls when OpenGL resources can 
//                     be used to create a drawing HANDLE.  
//Author: Andrew Olivier aolivier@ur.rochester.edu

//local includes
#include "gl/metadata/TreeNode.cpp"
#include "gl/metadata/Column.cpp"
#include "gl/objects/VAO.h"

namespace ctrl
{
  template <class HANDLE>
  class SceneModel
  {
    public:
      SceneModel(ColumnModel& cols): fTopLevelNodes(), fVAO(), fCols(cols)
      {
      }

      virtual ~SceneModel() = default;

      //User interface to a TreeNode's Row and handle for creating a new Row.  
      //A view is only valid as long as the SceneModel it was created from exists.  
      //So, don't try to cache views between events!  
      //TODO: view doesn't depend on SceneModel at all, so it could get its own file.
      class view
      {
        public:
          view(TreeNode<std::unique_ptr<HANDLE>>& node, ColumnModel& cols, mygl::VAO::model& vao): fNode(node), fCols(cols), fVAO(vao) {}
          virtual ~view() = default;
                                                                                                            
          template <class T>
          T& operator [](const Column<T>& col) 
          {
            return fNode.row[col];
          }

          template <class T, class ...ARGS> //T shall be a class derived from HANDLE.  ARGS shall 
                                            //be the arguments to a constructor for T.
          view emplace(const bool drawByDefault, ARGS... args)
          {
            fNode.children.emplace_back(std::unique_ptr<HANDLE>(new T(fVAO, args...)), fCols);
            auto& node = fNode.children.back();
            node.fVisible = drawByDefault;
            return view(node, fCols, fVAO);
          }
                                                                                                            
        private:
          TreeNode<std::unique_ptr<HANDLE>>& fNode; //This view exposes fNode.row and can be used to construct a TreeNode.  
          ColumnModel& fCols; //ColumnModel used to create children
          mygl::VAO::model& fVAO; //VAO of the SceneModel to which this view refers.
      };

      //Add a top-level TreeNode to this SceneModel which has no associated Drawable.  Useful for organizing 
      //Drawable-controlling TreeNodes.  
      view emplace(const bool drawByDefault)
      {
        fTopLevelNodes.emplace_back(nullptr, fCols);
        auto& node = fTopLevelNodes.back();
        node.fVisible = drawByDefault;
        return view(node, fCols, fVAO);
      }

      friend class SceneController; //Allow SceneController to access protected members of SceneModel so that 
                                    //user can set up SceneModel without being able to do OpenGL rendering.  

    protected:
      std::list<TreeNode<std::unique_ptr<HANDLE>>> fTopLevelNodes; //Top-level Nodes
      //TODO: The type of vertex in VAO should depend on HANDLE.  So, make VAO a class template.  
      mygl::VAO::model fVAO; //List of vertices that fDrawables need to access for rendering
      ColumnModel& fCols; //Reference to ColumnModel used to construct children
  };
} 
