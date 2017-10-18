//File: Accessor.h
//Brief: Abstract base class defining how information is retrieved from some event 
//       store.  View-derived classes will request 
//       data for drawing from an Accessor-derived class.  An EventController-derived 
//       class will update the Accessor's source of information and inform the 
//       Accessor that it needs to be updated.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//gtkmm include
#include <gtkmm.h>

#ifndef MODEL_ACCESSOR_H
#define MODEL_ACCESSOR_H

namespace view
{
  struct VisID;
}

namespace model
{
  class Accessor
  {
    public:
      Accessor();
      virtual ~Accessor() = default;
   
      //Provide application data 
      template <class T>
      bool Get(const T& info); 
      
    protected:
      Glib::RefPtr<Gtk::TreeStore> fRefGeoTree;
      Glib::RefPtr<Gtk::TreeStore> fRefEvtTree;
  };
}  

#endif //MODEL_ACCESSOR_H
