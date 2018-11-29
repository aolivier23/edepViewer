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

//imgui includes
#include "imgui.h"

//util includes
#include "util/GenException.h"

//c++ includes
#include <string>
#include <array>
#include <list>
#include <iostream>

#ifndef MYGL_USERCUT_H
#define MYGL_USERCUT_H

namespace ctrl
{
  class Row;
}

namespace mygl
{
  class UserCut
  {
    public:
      UserCut(const size_t nCols);
      virtual ~UserCut() = default;

      //Render this cut bar and apply its result
      //to the tree that starts at this list of 
      //root NODEs.  
      template <class NODE> //NODE shall have members fVisible and row 
      void Render(std::list<NODE>& root)
      {
        //Cut bar
        if(ImGui::InputText("##Cut", fBuffer.data(), fBuffer.size(), ImGuiInputTextFlags_EnterReturnsTrue)) 
        {
          fInput = fBuffer; //TODO: Do I need to do this copy now that everything is local to UserCut?
          ApplyCut(root);
        }
                                                                                                                                         
        if(ImGui::IsItemHovered())
        {
          ImGui::BeginTooltip();
          ImGui::Text("Apply cuts based on metadata associated\n"
                      "with each item in this list tree.");
          ImGui::BulletText("1 row <=> 1 drawn object");
          ImGui::BulletText("Refer to metadata rows with @<row number>.\n"
                            "So, @0 refers to the names of objects in\n"
                            "the Grids scene.");
          ImGui::BulletText("Comparison operators < and > are allowed for\n"
                            "numbers.  So, if the second row from the left\n"
                            "of a scene is a number representing Energy,\n"
                            "@1 < 100 would stop drawing all rows with less\n"
                            "than 100 units of Energy.");
          ImGui::BulletText("Only == and != are supported for strings.");
          ImGui::BulletText("Please enclose sub-expressions in ().  You can\n"
                        "combine subexpressions with && (and) and || (or),\n"
                        "and you can even compare string comparisons with\n"
                        "number comparisons like (@2 < 100) && (@1 == neutron)");
          ImGui::BulletText("The expression compiler tries to automatically\n"
                            "determine whether the LHS and RHS of each\n"
                            "sub-expression are strings or numbers.  If\n"
                            "you try to compare a string to a number, the\n"
                            "cut bar will stop processing rows wherever it\n"
                            "first encountered a problem and print an error\n"
                            "message to STDOUT.");
          ImGui::EndTooltip();
        }
      }
 
      //Just apply cuts, but don't render a GUI.  Publicly useful to "remember" cuts immediately 
      //after loading a new event.  
      template <class NODE>
      void ApplyCut(std::list<NODE>& root)
      {
        //Turn off drawing for 3D objects whose metdata don't pass cut
        try
        {
          //Don't cut on top-level nodes because they're understood to be placeholders that aren't associated with Drawables anyway.  
          //TODO: The above comment seems to violate the idea of a tree model that I want users to work with.  Consider revising 
          //      the idea of "placeholder nodes".  Just forcing the user to use Noop Drawable for placeholder nodes might be 
          //      slightly better.
          for(auto& top: root) //TODO: Checkbox to cut on top-level nodes?
                               //TODO: Option for recursively applying cuts to children?
          {
            //The lambda function below does the job that apply_filter() used to do.  Nodes aren't reordered based on 
            //visibility anymore to speed up cut bar processing.  
            for(auto& child: top.children)
            {
              child.walkIf([this](auto& node)
                           {
                             //if(!node.fVisible) return false;
                             return (node.fVisible = this->do_filter(node.row));
                           });
            }
          }
        }
        catch(const util::GenException& e)
        {
          std::cerr << "Caught exception during formula processing:\n" << e.what() << "\nIgnoring cuts for this SceneController.\n";
        }
      }

    protected:
      bool do_filter(const ctrl::Row& row);

      //Functions that implement cut
      bool ev(std::string expr);
      std::string subexpr(std::string expr);
      std::string strip_spaces(std::string expr);

      //Data accumulated by UserCut
      size_t fNCols; //Number of columns to process

      //GUI data
      static constexpr int fBufferDepth = 256;
      std::array<char, fBufferDepth> fInput; //User-supplied text to cut based on
      std::array<char, fBufferDepth> fBuffer; //Cut bar buffer
  };
}

#endif //MYGL_USERCUT_H
