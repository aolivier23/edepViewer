//File: Vec3Entry.cpp
//Brief: A Vec3Entry allows the user to edit the components of a glm::vec3 with a 
//       gtkmm widget.
//Author: Andrew Olivier aolivier@ur.rochester.edu 

//local includes
#include "gl/camera/Vec3Entry.h"

namespace mygl
{
  Vec3Entry::Vec3Entry(const std::string& name, const glm::vec3& init): Gtk::Box(Gtk::ORIENTATION_VERTICAL), fLabel(name), fXLabel("x"), fXEntry(), 
                                                                        fYLabel("y"), fYEntry(), fZLabel("z"), fZEntry()
  {
    //Forward components' activate signals as this class' activate signal
    fXEntry.signal_activate().connect(sigc::mem_fun(*this, &Vec3Entry::do_activate));
    fYEntry.signal_activate().connect(sigc::mem_fun(*this, &Vec3Entry::do_activate));
    fZEntry.signal_activate().connect(sigc::mem_fun(*this, &Vec3Entry::do_activate));

    //Set default values
    fXEntry.set_text(std::to_string(init.x));
    fYEntry.set_text(std::to_string(init.y));
    fZEntry.set_text(std::to_string(init.z));

    pack_start(fLabel, Gtk::PACK_SHRINK);
    pack_start(fXLabel, Gtk::PACK_SHRINK);
    pack_start(fXEntry, Gtk::PACK_SHRINK);
    pack_start(fYLabel, Gtk::PACK_SHRINK);
    pack_start(fYEntry, Gtk::PACK_SHRINK);
    pack_start(fZLabel, Gtk::PACK_SHRINK);
    pack_start(fZEntry, Gtk::PACK_SHRINK);
  }

  glm::vec3 Vec3Entry::get_value() const
  {
    const auto x = std::stof(fXEntry.get_text());
    const auto y = std::stof(fYEntry.get_text());
    const auto z = std::stof(fZEntry.get_text());
    return glm::vec3(x, y, z);
  }

  void Vec3Entry::set_value(const glm::vec3& values)
  {
    fXEntry.set_text(std::to_string(values.x));
    fYEntry.set_text(std::to_string(values.y));
    fZEntry.set_text(std::to_string(values.z));
  }

  sigc::signal<void> Vec3Entry::signal_activate()
  {
    return fSignalActivate;
  }

  void Vec3Entry::do_activate()
  {
    fSignalActivate.emit();
  }
}
