//File: Vec3Entry.h
//Brief: A Vec3Entry is a Gtk::Box with three fields to edit the entries of a glm::vec3.  
//Author: Andrew Olivier aolivier@ur.rochester.edu

//gtkmm includes
#include <gtkmm.h>

//glm includes
#include <glm/glm.hpp>

//c++ includes
#include <string>

#ifndef MYGL_VEC3ENTRY_H
#define MYGL_VEC3ENTRY_H

namespace mygl
{
  class Vec3Entry: public Gtk::Box
  {
    public:
      Vec3Entry(const std::string& name, const glm::vec3& init = glm::vec3(0.f, 0.f, 0.f));
      virtual ~Vec3Entry() = default;

      glm::vec3 get_value() const;

      void set_value(const glm::vec3& values);

      sigc::signal<void> signal_activate();

    protected:
      Gtk::Label fLabel; //Displays the parameter passed as "name" to this class' constructor
      Gtk::Label fXLabel;
      Gtk::Entry fXEntry;
      Gtk::Label fYLabel;
      Gtk::Entry fYEntry;
      Gtk::Label fZLabel;
      Gtk::Entry fZEntry;

      //Signal for one or more components activated.  This means that the activated 
      //component's widget has detected that enter was pressed while it had focus(?).  
      sigc::signal<void> fSignalActivate;

      //Make sure fSignalActivate is emitted when any of this widget's entries 
      //is activated.
      void do_activate();
  };
}

#endif //MYGL_VEC3ENTRY_H
