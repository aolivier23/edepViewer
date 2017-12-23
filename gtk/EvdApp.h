//File: EvdApp.h
//Brief: An EvdApp is a Gtk::Application that knows how to forward configuration and input files to an EvdWindow.  It's main job is to 
//       handle command line input for now. 
//Author: Andrew Olivier

//local includes
#include "gtk/EvdWindow.h"

//gtkmm include
#include <gtkmm.h>

namespace mygl
{
  class EvdApp: public Gtk::Application
  {
    public:
      EvdApp();
      virtual ~EvdApp() = default;

      static Glib::RefPtr<EvdApp> create();

      virtual void on_open(const std::vector<Glib::RefPtr<Gio::File>>& files, const Glib::ustring& hints) override;
      virtual void on_activate() override;

    protected:
      std::unique_ptr<EvdWindow> fWindow;

      
  };
} 
