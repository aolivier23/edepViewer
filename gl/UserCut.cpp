//File: UserCut.cpp
//Brief: A Gtk::Entry that provides a do_filter() function for Gtk::TreeModelFilter to 
//       filter entries based on user input.  See header for documentation of cut syntax.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//gl includes
#include "gl/UserCut.h"
#include "gl/model/GenException.h"

//c++ includes
#include <string>
#include <regex>

namespace
{
  template <class T>
  std::string get_from_col(const int col, const Gtk::TreeRow& row)
  {
    T value;
    row.get_value(col, value);
    return std::to_string(value); 
  }
}

namespace mygl
{
  UserCut::UserCut(Gtk::TreeModelColumnRecord& cols, const std::string& formula): fCols(cols), 
                   fTokenize(r"(\(.*\)|[[:digit:]]*|[[:alpha:]]|\|\||&&|<=|>=|<|>|==|!)+"), 
                                                                                  fIsAlpha("[[:alpha:]]*")
  {
    set_text(formula);
  }

  bool UserCut::do_filter(const Gtk::TreeRow::iterator& iter) const
  {
    TFormula eval("UserCut", get_text().c_str(), 0, fCols.size(), false);
    //TODO: Figure out whether TFormula will evaluate expressions with text comparison and finish this function
    eval.
  }

  //TODO: This is horrible!  Is there a better semi-typesafe way to do this?  
  std::string UserCut::get_col_value(const int col, const Gtk::TreeRow& row)
  {
    GType colType = (fCols.types())[col];
    std::string typeName(g_type_name(colType)); //VERY bad example
    //Dispatch to proper function if type is know.  If not, throw an exception. 
    //TODO: I could probably write some TMP trickery to hide even this dispatch function...
    if(typeName == "gchararray") return get_from_col<std::string>(col, row);
    if(typeName == "gdouble") return get_from_col<double>(col, row);
    if(typeName == "gboolean") return get_from_col<bool>(col, row);
    //TODO: The documentation at https://developer.gnome.org/glib/stable/glib-Basic-Types.html claims these exist...
    if(typeName == "gint") return get_from_col<int>(col, row);
    if(typeName == "guint") return get_from_col<unsigned int>(col, row);
    if(typeName == "gshort") return get_from_col<short>(col, row);
    if(typeName == "gushort") return get_from_col<unsigned short>(col, row);
    if(typeName == "gfloat") return get_from_col<float>(col, row);
    throw util::GenException("Unrecognized Type") << "In mygl::UserCut::get_col_value(), got column of type " 
                                                  << colType << " that is not currently handled.  If you feel very "
                                                  << "strongly about using this type, update this function.\n";
    return "NULL";
  }
}
