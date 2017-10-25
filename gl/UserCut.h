//File: UserCut.h
//Brief: A UserCut is a Gtk::Entry that builds a formula from user input for filtering a 
//       Gtk::TreeModel via Gtk::TreeModelFilter's set_filter_func() interface.  To set up a UserCut, 
//       user sigc::mem_fun(userCutPointer, &UserCut::do_filter) as the parameter to Gtk::TreeModelFilter::set_filter_func().
//
//       Warning: Since UserCut uses Gtk::TreeRow::get_value(int col, ColumnType& value), it 
//                can fail at runtime.  If in debug mode, you will likely get several Gtk 
//                assertion failures before your program crashes.  This seems to be the 
//                nature of allowing the user to access arbitrary columns. 
//
//       Syntax for "user code" that is typed into the Gtk::Entry:
//       @<number>: Value in column <number>
//       lhs < rhs: True if lhs is less than rhs.  False otherwise.  
//       lhs > rhs: True if lhs is greater than rhs.  False otherwise. 
//       lhs == rhs: True is lhs is equal to rhs.  False otherwise.
//       lhs <= rhs: True if lhs is less than or equal to rhs.  False otherwise.
//       lhs >= rhs: True is lhs is greater than or equal to rhs.  False otherwise.
//       first && second: True if first is true and second is true.  False otherwise.
//       first || second: True if first is true or if second is true.  False otherwise.  
//       !first: True if first is false and false if first is true.  
//       first && (second || third): True if first is true and:
//                                   second is true
//                                   OR
//                                   third is true
//       first && second || third: True if first and second are true OR third is true
//Author: Andrew Olivier aolivier@ur.rochester.edu

//gtkmm include
#include <gtkmm.h>

#ifndef MYGL_USERCUT_H
#define MYGL_USERCUT_H

namespace mygl
{
  class UserCut: public Gtk::Entry
  {
    public:
      UserCut(Gtk::TreeModel::ColumnRecord& cols, const std::string& initial = "true");
      virtual ~UserCut() = default;

      bool do_filter(const Gtk::TreeModel::const_iterator& iter);
    
    protected:
      std::vector<GType> fTypes; //observer pointer
      const size_t fNTypes;

      bool ev(std::string expr);
      std::string subexpr(std::string expr);
      std::string strip_spaces(std::string expr);
      

    private:
      std::string get_col_value(const int col, const Gtk::TreeRow& row);
  };
}

#endif //MYGL_USERCUT_H
