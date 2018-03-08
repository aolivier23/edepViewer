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
          size_t size() const;

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

          void SetPosition(const size_t pos); //TODO: friend function of ColumnModel?

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

          //Access to Node data using Columns
          template <class T>
          T& operator[](Column<T>& col); 

          //Access to Node data as strings
          std::string operator [](const size_t index);
 
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
          void Remove(iterator child); //Remove a child of this Node
          void Reparent(Node* parent); //Change the parent of this Node.  Parameter is an observer pointer.
  
        private:
          //Node* fParent; //Observer pointer to parent.  Think about what happens in parent's destructor...
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
              virtual std::string string() = 0; //Turn whatever data is stored into a std::string
 
              //TODO: virtual function to put DataBase() information on the GPU?
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
              
              //Turn data into a string.  Probably called std::to_string for built-in types.
              virtual std::string string() = 0;

            protected:
              T fData; //The actual data stored in a DataBase
          };
      };

      class iterator
      {
        public:
          iterator(Base& node); //TODO: Pass parent or child Node here?
          ~iterator() = default;

          //Node access
          Node& operator *() const;
          Node* operator ->() const;

          //Minimal STL-style iterator interface
          iterator& operator ++();
          iterator& operator ++(int);
          operator (bool)() const;
          //iterator Parent(); //TODO: Automatically go to fParent's next child in operator ++ instead?
          bool operator <(iterator& other) const; 
          bool operator ==(iterator& other) const;

        private:
          typedef Base std::vector<std::unique_ptr<Node>>::iterator;
          Base fNode; //Access to the Node this iterator references
          Base fParent; //Access to the parent of the Node this iterator references
      };

      class const_iterator
      {
        public:
          iterator(const Base& node); //TODO: Pass parent or child Node here?
          ~iterator() = default;

          //Node access
          const Node& operator *() const;
          const Node* operator ->() const;

          //Minimal STL-style iterator interface
          const_iterator& operator ++();
          const_iterator& operator ++(int);
          operator (bool)() const;
          //iterator Parent(); //TODO: Automatically go to fParent's next child in operator ++ instead?
          bool operator <(const_iterator& other) const;
          bool operator ==(const_iterator& other) const;

        private:
          typedef Base std::vector<std::unique_ptr<Node>>::iterator;
          Base fNode; //Access to the Node this iterator references
          Base fParent; //Access to the parent of the Node this iterator references
      };

      //Public interface of core TreeModel
      TreeModel(const ColumnModel& cols); //TODO: Can I get away with a const ColumnModel?
      virtual ~TreeModel() = default;

      //Operations on the Node hierarchy
      iterator NewNode(); //Create a new top-level Node
      iterator NewNode(iterator parent); //Add a child Node to the Node "pointed" by parent
      void Remove(iterator child); //Remove child from its' parent Node and destroy child 
      void Clear(); //Delete all of the Nodes in this TreeModel

      //Access to top-level nodes
      iterator begin();
      iterator end();
      const_iterator cbegin() const;
      const_iterator cend() const;

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

      //Call a callable object on every Node in this Tree
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
      std::vector<std::unique_ptr<Node>> fTopNodes; //top-level nodes
  };
} 
