//File: TreeModel.h
//Brief: A TreeModel stores arbitrary user data in columns for later display.  Users can retrieve Nodes whose 
//       access operator can be used to access the column value for each entry.  Interface design heavily 
//       influenced by gtkmm's Gtk::TreeModel. 
//Author: Andrew Olivier aolivier@ur.rochester.edu

//c++ includes
#include <list>
#include <vector>
#include <memory>
#include <sstream>

#ifndef MYGL_TREEMODEL_H
#define MYGL_TREEMODEL_H

namespace mygl
{

  //TODO: Make this a class template and move Node to a separate file?
  class TreeModel
  {
    public:

      class ColumnModel;
      template <class T> class Column;
      class iterator;
      class const_iterator;

      //One entry in a TreeModel.  User can retrieve and set values with operator[] using the columns of this tree.  
      //Knows about its' parent and owns its' children.  
      //TODO: Separate concepts of iterator and Node.  An iterator holds a position in another Node's list of children 
      //      as well as an iterator(?) to the parent Node.  
      //TODO: Isn't this similar to boost::any?  
      class Node
      {
        public: //TODO: Make this private again by restructing ColumnModel?
          //Internal details of how a Node stores and accesses data.  
          //Base class for data access.  Allows (mostly) arbitrary types to be 
          //stored and destroyed correctly.
          class DataBase
          {
            public:
              virtual ~DataBase() = default;
                                                                                                                        
              virtual void* Get() = 0; //Type will be figured out by Column<T>'s T
              virtual std::string string() = 0; //Turn whatever data is stored into a std::string
                                                                                                                        
              //TODO: virtual function to put DataBase() information on the GPU?
          };

        public:
          Node(const ColumnModel& cols);
          virtual ~Node() = default;

          //Explicitly delete copy facilities
          Node(const Node& other) = delete;
          Node& operator=(Node other) = delete;
                                                                                                                        
          //Access to Node data using Columns
          template <class T>
          T& operator[](Column<T>& col)
          {
            return *((T*)(fColData[col.GetPosition()]->Get()));
          }
                                                                                                                        
          //Access to Node data as strings
          std::string operator [](const size_t index);
                                                                                                                        
          //Access to parentage
          inline iterator begin() { return iterator(fChildren.begin()); }
          inline iterator end() { return iterator(fChildren.end()); }
          inline const_iterator cbegin() const { return const_iterator(fChildren.cbegin()); }
          inline const_iterator cend() const { return const_iterator(fChildren.cend()); }
          size_t NChildren() const { return fChildren.size(); }
                                                                                                                        
          //Alter parentage
          inline iterator NewChild(const ColumnModel& cols) 
          { 
            fChildren.emplace_back(new Node(cols));
            return iterator(--(fChildren.end())); 
          }
          inline void Remove(iterator child) { fChildren.erase(child.GetBase()); } //Remove a child of this Node
          inline void MoveToEnd(iterator child) { fChildren.splice(fChildren.end(), fChildren, child.GetBase()); } 
          //Move a child to the end of this Node
                                                                                                                        
        private:
          std::list<std::unique_ptr<Node>> fChildren; //Owning pointers to children.
          std::vector<std::unique_ptr<DataBase>> fColData; //Vector of column data
         
        public: 
          //(I think I got this right) Store actual data as a real type so I can delete it. 
          //Cast it to a void* and then back so that I can actually write a sane user interface.    
          template <class T>
          class Data: public DataBase
          {
            public:
              virtual ~Data() = default;
                                                                                                                        
              //Cast fData to void* so it can be cast back later.  
              virtual void* Get() override { return (void*)(&fData); }
              
              //Turn data into a string.  Using operator << instead of std::to_string() to support
              //VisID with minimal work.  
              virtual std::string string() override 
              { 
                std::stringstream ss;
                ss << fData;
                return ss.str();
              }
                                                                                                                        
            protected:
              T fData; //The actual data stored in a DataBase
          };
      };

      //A ColumnBase provides a way to hold a collection of objects that each know something about a specific type. 
      //ColumnBase also provides the user with a handle to the type of object in each column.  
      //In the future, I am considering sending specific columns to the GPU.  I think ColumnBase might need to know 
      //the type of the object it is holding then.
      class ColumnBase
      { 
        public:
          ColumnBase(const std::string& name);
          virtual ~ColumnBase() = default;
                                                                                                                     
          virtual std::unique_ptr<Node::DataBase> BuildData() const = 0;
                                                                                                                     
          void SetPosition(const size_t pos) { fPosition = pos; } //TODO: friend function of ColumnModel?
          inline size_t GetPosition() const { return fPosition; }

          inline std::string Name() const { return fName; }                                                                                                                     
        private: 
          std::string fName; //The name of this column when a TreeModel using it is drawn.
          size_t fPosition; //The position of this ColumnBase in the column model (note the lack of capitalization)
      };
                                                                                                                     
      //Concrete implementation of ColumnBase. Encodes the type information for the object it refers to so that 
      //Node can extract that information with a minimum of trouble.  
      template <class T>
      class Column: public ColumnBase
      {
        public:
          Column(const std::string& name): ColumnBase(name) {}
          virtual ~Column() = default;
                                                                                                                     
          virtual std::unique_ptr<Node::DataBase> BuildData() const override { return std::unique_ptr<Node::DataBase>(new Node::Data<T>()); }
      };

      //The list of columns in a TreeModel.  Users should derive from this class to use it.  
      //Give your derived class Column<T> members, and call Add() on each column in the constructor.  
      class ColumnModel
      {
        public:
          void Add(ColumnBase& col); //A ColumnBase must be Add()ed to ColumnModel to show up in trees that use a given model
          //TODO: Set fPosition here somehow.  Class friendship?  Something more devious?  
          //      Already figured out I can't create the ColumnBases myself in this class 
          //      because the user needs access to the Column<T>s themselves for Node::operator[].
          size_t size() const;
        
          std::vector<std::unique_ptr<Node::DataBase>> BuildData() const;

          inline std::string Name(const size_t col) const { return fCols[col]->Name(); }

        protected:
          ColumnModel() {} //Derive from this class to use it.  

        private:
          std::vector<ColumnBase*> fCols;
      };

      class iterator
      {
        private:
          typedef std::list<std::unique_ptr<Node>>::iterator Base;

        public:
          iterator(Base node); 
          ~iterator() = default;

          //Node access
          Node& operator *() const;
          Node* operator ->() const;

          //Minimal STL-style iterator interface
          iterator operator ++();
          iterator operator ++(int);

          //iterator operator +(const int dist);
          //bool operator <(const iterator& other) const; 
          bool operator ==(const iterator& other) const;
          bool operator !=(const iterator& other) const { return !(*this == other); }

          //Access to vector iterator
          inline Base& GetBase() { return fNode; }

        private:
          Base fNode; //Access to the Node this iterator references
          //TODO: Singly or double linked?
          //Base fParent; //Access to the parent of the Node this iterator references
      };

      class const_iterator
      {
        private:
          typedef std::list<std::unique_ptr<Node>>::const_iterator Base;

        public:
          const_iterator(const Base node); //TODO: Pass parent or child Node here?
          ~const_iterator() = default;

          //Node access
          const Node& operator *() const;
          const Node* operator ->() const;

          //Minimal STL-style iterator interface
          const_iterator operator ++();
          const_iterator operator ++(int);
          //bool operator <(const const_iterator& other) const;
          bool operator ==(const const_iterator& other) const;
          bool operator !=(const const_iterator& other) const { return !(*this == other); }

          //Access to vector iterator
          inline Base& GetBase() { return fNode; }

        private:
          Base fNode; //Access to the Node this iterator references
          //TODO: Singly or double linked?
          //Base fParent; //Access to the parent of the Node this iterator references
      };

      //Public interface of core TreeModel
      TreeModel(const std::shared_ptr<ColumnModel> cols): fColumns(cols), fTopNodes() {}
      virtual ~TreeModel() = default;

      //Operations on the Node hierarchy
      //TODO: These functions seem to indicate that TreeModel itself should be derived from Node
      inline iterator NewNode() 
      { 
        fTopNodes.emplace_back(new Node(*fColumns)); 
        return iterator(--(fTopNodes.end())); 
      } //Create a new top-level Node
      inline iterator NewNode(const iterator& parent)
      {
        return parent->NewChild(*fColumns);
      }

      inline void Remove(iterator top) { fTopNodes.erase(top.GetBase()); } //Remove child from its' parent Node and destroy child 
      inline void Clear() { fTopNodes.clear(); } //Delete all of the Nodes in this TreeModel

      //Access to top-level nodes
      inline iterator begin() { return iterator(fTopNodes.begin()); }
      inline iterator end() { return iterator(fTopNodes.end()); }
      inline const_iterator cbegin() const { return const_iterator(fTopNodes.cbegin()); }
      inline const_iterator cend() const { return const_iterator(fTopNodes.cend()); }

      //Convenience functions for walking all of the Nodes in this TreeModel
      //Call a callable object on each descendant of parent
      //TODO: Doesn't need to be a member function at all 
      //TODO: const implementation?  Code would look ridiculous, but I think it 
      //      would allow the user to still Walk() a const TreeModel as long as 
      //      func takes Nodes as const through an implicit requirement.
      template <class FUNC>
      void Walk(iterator parent, FUNC&& func)
      {
        for(auto child = parent->begin(); child != parent->end(); ++child)
        {
          func(child);
          Walk(child, func);
        }
      }

      //Call a callable object on every Node in this Tree
      template <class FUNC>
      void Walk(FUNC&& func)
      {
        for(auto node = this->begin(); node != this->end(); ++node)
        {
          func(node);
          Walk(node, func);
        }
      }

    protected:
      std::shared_ptr<ColumnModel> fColumns; //Remember the columns in this TreeModel so that someone else can draw them.
                                             //The user should still own his own copy(ies) of this ColumnModel.  
      std::list<std::unique_ptr<Node>> fTopNodes; //top-level nodes
  };
}

#endif //MYGL_TREEMODEL_H 
