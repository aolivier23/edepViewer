//File: TreeNode.cpp
//Brief: A TreeNode associates a Row of metadata with a HANDLE and may manage child TreeNodes.  
//Author: Andrew Olivier aolivier@ur.rochester.edu

//Local includes
#include "Row.h"
#include "gl/selection/VisID.h"

//c++ includes
#include <list>
#include <algorithm>

#ifndef MYGL_TREENODE_CPP
#define MYGL_TREENODE_CPP

namespace ctrl
{
  //HANDLE shall be move-assignable and expose a pointer-like interface to an object that can be Draw()n.   
  template <class HANDLE>
  struct TreeNode
  {
    TreeNode(HANDLE&& toCopy, ColumnModel& cols): children(), row(cols), handle(std::move(toCopy)) {}
    virtual ~TreeNode() = default;

    //TODO: If I'm using all 3 walk functions a lot, I could clean up this interface by overloading for 
    //      tags instead of using different function names and maybe an enable_if() for when func() 
    //      returns a bool.  
    //Recursively call a function for each descendant of this TreeNode.  
    //FUNC is a callable object that takes a TreeNode as argument whose return value is ignored.  
    template <class FUNC>
    void walk(FUNC&& func)
    {
      func(*this);
      for(auto& child: children) child.walk(func);
    }

    //Same as walk() except func() takes const arguments.
    template <class FUNC>
    void walk(FUNC&& func) const
    {
      func(*this);
      for(const auto& child: children) child.walk(func);
    }

    //Recursively call a function for each descendant of this TreeNode if that function returns true
    //FUNC is a callable object that takes a TreeNode and returns true if its children should be walked
    template <class FUNC>
    void walkIf(FUNC&& func)
    {
      if(func(*this)) for(auto& child: children) child.walkIf(func);
    }

    //Same as above except that func() takes a const TreeNode
    template <class FUNC>
    void walkIf(FUNC&& func) const
    {
      if(func(*this)) for(const auto& child: children) child.walkIf(func);
    }

    //Recursively call a function for each descendant of this TreeNode if that function returns	true
    //BEFORE is a callable object that takes a TreeNode and returns true if its children should be walked
    //AFTER is a callable object that takes a TreeNode whose return value is ignored.  It is only called 
    //      if BEFORE returned true.
    template <class BEFORE, class AFTER>
    void walkIf(BEFORE&& before, AFTER&& after)
    {
      if(before(*this)) 
      {
        for(auto& child: children) child.walkIf(before, after);
        after(*this);
      }
    }

    //Same as above, but HANDLE argument to func() is const
    template <class BEFORE, class AFTER>
    void walkIf(BEFORE&& before, AFTER&& after) const
    {
      if(before(*this)) 
      {
        for(const auto& child: children) child.walkIf(before, after);
        after(*this);
      }
    }
   
    //Recursively call a function for each descendant as long as that function returns true.  
    //The first time func() returns false on a child, stop looping over children.
    template <class FUNC>
    bool walkWhileTrue(FUNC&& func)
    {
      if(!func(*this)) return false;
      for(auto& child: children) if(!child.walkWhileTrue(func)) break;
      return true;
    }

    //Same as walkWhileTrue except func() takes const parameters
    template <class FUNC>
    bool walkWhileTrue(FUNC&& func) const
    {
      if(!func(*this)) return false;
      for(const auto& child: children) if(!child.walkWhileTrue(func)) break;
      return true;
    }

    //Recursively perform a binary search for id, and call func() on anything that could be in the path to id.  
    //Returns whether id was found.
    template <class FUNC> //FUNC is any callable object whose return value is ignored
    bool search(FUNC&& func, const mygl::VisID& id)
    {
      if(this->fVisID == id) //If this is the node we're searching for
      {
        func(*this);
        return true;
      }

      //Search deeper
      //TODO: Be careful, I'm allocating an object on the stack in a recusive function.  This memory could add up in deep recursion.
      const auto upper = std::upper_bound(children.begin(), children.end(), id, 
                                          [](const mygl::VisID& id, const auto& child) { return id < child.fVisID; });

      if(upper != children.begin()) //If lower == children.cend(), then one of the children of the last child of this node could still be a match
      {
        if(std::prev(upper)->search(func, id))
        {
          func(*this);
          return true;
        }
      }
      return false; 
    }

    //Same as search, but func() takes a const node.
    template <class FUNC> //FUNC is any callable object whose return value is ignored
    bool search(FUNC&& func, const mygl::VisID& id) const
    {
      if(this->fVisID == id) //If this is the node we're searching for
      {
        func(*this);
        return true;
      }
                                                                                                                                                  
      //Search deeper
      //TODO: Be careful, I'm allocating an object on the stack in a recusive function.  This memory could add up in deep recursion.
      const auto upper = std::upper_bound(children.begin(), children.end(), id, 
                                          [](const mygl::VisID& id, const auto& child) { return id < child.fVisID; });
                                                                                                                                                  
      if(upper != children.begin()) //If lower == children.cend(), then one of the children of the last child of this node could still be a match
      {
        if(std::prev(upper)->search(func, id))
        {
          func(*this);
          return true;
        }
      }
      return false; 
    }

    //Interface that SceneController will use
    std::list<TreeNode<HANDLE>> children; //Children of this TreeNode
    Row row; //Metadata associated with a HANDLE
    HANDLE handle; //HANDLE for controlling drawing
    bool fVisible; //Whether this TreeNode is visible
    mygl::VisID fVisID; //Identifier associated with this row to make it selectable
  };
}

#endif //MYGL_TREENODE_CPP 
