//File: TreeModel.h
//Brief: A TreeModel stores arbitrary user data in columns for later display.  Users can retrieve Nodes whose 
//       access operator can be used to access the column value for each entry.  Interface design heavily 
//       influenced by gtkmm's Gtk::TreeModel. 
//Author: Andrew Olivier aolivier@ur.rochester.edu

namespace mygl
{
  class TreeModel
  {
    public:

      //The list of columns in a TreeModel.  Users should derive from this class to use it.  
      //Give your derived class Column<T> members, and call Add() on each column in the constructor.  
      class ColumnModel
      {
        public:
          void Add(ColumnBase& col); //A ColumnBase must be Add()ed to ColumnModel to show up in trees that use a given model
          //TODO: Set fPosition here somehow.  Class friendship?  Something more devious?  
          //      Already figured out I can't create the ColumnBases myself in this class 
          //      because the user needs access to the Column<T>s themselves for Node::operator[].

        protected:
          ColumnModel() {} //Derive from this class to use it.  
          std::vector<std::unique_ptr<ColumnBase>> fCols;
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

          virtual std::unique_ptr<Node::DataBase> BuildData() = 0;

        private: 
          std::string fName; //The name of this column when a TreeModel using it is drawn.
          //TODO: Using fPosition requires a little less memory but more moving parts.  Using a std::map for now instead so I 
          //      can get this system working.  
          //size_t fPosition; //The position of this ColumnBase in the column model (note the lack of capitalization)
      };

      //Concrete implementation of ColumnBase. Encodes the type information for the object it refers to so that 
      //Node can extract that information with a minimum of trouble.  
      template <class T>
      class Column: public ColumnBase
      {
        public:
          ColumnBase(const std::string& name);
          virtual ~ColumnBase() = default;

          virtual std::unique_ptr<Node::DataBase> BuildData() override;
      };

      //One entry in a TreeModel.  User can retrieve and set values with operator[] using the columns of this tree.  
      //Knows about its' parent and owns its' children.  
      //TODO: Separate concepts of iterator and Node.  An iterator holds a position in another Node's list of children 
      //      as well as an iterator(?) to the parent Node.  
      class Node
      {
        public:
           Node(ColumnModel& cols);
           virtual ~Node() = default;

           template <class T>
           T& operator[](Column<T>& col); 

           //Access to parentage
           //TODO: Having to dereference an iterator twice seems like an ugly interface.  
           //      Maybe write a wrapper over vector iterators that just returns a Node& on 
           //      dereference.
           Node& Parent(); 
           iterator child_begin();
           iterator child_end();
           const_iterator child_cbegin() const;
           const_iterator child_cend() const;

           //Alter parentage
           //iterator Child(); //Create a new child of this Node and return an iterator to the child.
                               //Since Child() needs access to a ColumnModel, this is a function of the tree interface itself.  
           void Remove(iterator child); //Remove a child of this Node
           void Reparent(Node* parent); //Change the parent of this Node.  Parameter is an observer pointer.
  
        private:
          Node* fParent; //Observer pointer to parent.  Think about what happens in parent's destructor...
          std::vector<std::unique_ptr<Node>> fChildren; //Owning pointers to children.
          std::vector<std::unique_ptr<DataBase>> fColData; //Vector of column data
          
          //Internal details of how a Node stores and accesses data.  
          //Base class for data access.  Allows (mostly) arbitrary types to be 
          //stored and destroyed correctly.
          class DataBase
          {
            public:
              virtual ~DataBase() = default;

              virtual void* Get() = 0; //Type will be figured out by Column<T>'s T
 
              //TODO: virtual function to put DataBase() information on  the GPU?
          };

          //(I think I got this right) Store actual data as a real type so I can delete it. 
          //Cast it to a void* and then back so that I can actually write a sane user interface.    
          template <class T>
          class Data: public DataBase
          {
            public:
              virtual ~Data() = default;
 
              //Cast fData to void* so it can be cast back later.  
              virtual void* Get() override;

            protected:
              T fData; //The actual data stored in a DataBase
          };
      };

      class iterator
      {
        public:
          iterator(Base& node); //TODO: Pass parent or child Node here?
          ~iterator() = default;

          //Minimal STL-style iterator interface
          iterator& operator ++();
          iterator& operator ++(int);
          operator (bool)();
          iterator Parent(); //TODO: Automatically go to fParent's next child in operator ++ instead?
          bool operator <(iterator& other); 

        private:
          typedef Base std::vector<std::unique_ptr<Node>>::iterator;
          Base fNode; //Access to the Node this iterator references
          Base fParent; //Access to the parent of the Node this iterator references
      };

      //TODO: const_iterator?

      //Public interface of core TreeModel
      TreeModel(const ColumnModel& cols); //TODO: Can I get away with a const ColumnModel?
      virtual ~TreeModel() = default;

      //Operations on the Node hierarchy
      //TODO: iterator class
      iterator NewNode(); //Create a new top-level Node
      iterator NewChild(iterator parent); //Add a child Node to the Node "pointed" by parent
      void Remove(iterator child); //Remove child from its' parent Node and destroy child 

      //TODO: iterators to top-level nodes

      //Convenience functions for walking all of the Nodes in this TreeModel
      //Call a callable object on each descendant of parent
      //TODO: Doesn't need to be a member function at all 
      //TODO: const implementation?  Code would look ridiculous, but I think it 
      //      would allow the user to still Walk() a const TreeModel as long as 
      //      func takes Nodes as const through an implicit requirement.
      template <class FUNC>
      void Walk(iterator parent, FUNC& func)
      {
        for(auto& child: parent)
        {
          func(child);
          Walk(child, func);
        }
      }

      template <class FUNC>
      void Walk(FUNC& func)
      {
        for(auto& node: *this) 
        {
          func(node);
          Walk(node, func);
        }
      }

    protected:
      std::unique_ptr<ColumnModel> fColumns; //Remember the columns in this TreeModel so that someone else can draw them.
                                             //The user should still own his own copy(ies) of this ColumnModel.  
                                             //TODO: What happens if user's ColumnModel gets out of sync with this one?  
      std::vector<std::unique_ptr<Node>> fTopNodes; //top-level nodes
  };
} 
