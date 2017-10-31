//File: UserCut.cpp
//Brief: A Gtk::Entry that provides a do_filter() function for Gtk::TreeModelFilter to 
//       filter entries based on user input.  See header for documentation of cut syntax.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//gl includes
#include "gl/UserCut.h"
#include "gl/model/GenException.h"

//c++ includes
#include <string>
#include <iostream>
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

  template <>
  std::string get_from_col<gchararray>(const int col, const Gtk::TreeRow& row)
  {
    //gchararray chars;
    std::string chars;
    row.get_value(col, chars);

    /*if(chars != nullptr)
    {
      //Find out why I am getting empty strings
      char current = chars[0];
      size_t pos = 0;
      for(pos = 0; current != '\0'; ++pos)
      {
        current = chars[pos];
        std::cout << current << "\n";
      }
      std::cout << "\n";
      if(chars[0] == '\0') 
      {
        std::cout << "Got null character at position " << pos << "\n";
      }
      
      return std::string(chars);
    }
    else 
    {
      std::cerr << "Got a nullptr when trying to read characters for column " << col << "\n";
      return std::string("@"+std::to_string(col));
    }*/
    return chars;
  }
}

namespace mygl
{
  UserCut::UserCut(const std::string& formula)
  {
    set_text(formula);
  }

  void UserCut::SetTypes(std::vector<std::string> types)
  {
    fTypes = types;
  }

  bool UserCut::do_filter(const Gtk::TreeModel::iterator& iter) 
  {
    std::string copy = get_text();
    auto& row = *iter;

    //Do not cut out "root" nodes.  Could add some flag for "organization" nodes later, but having a heterogeneous 
    //set of objects in a Scene already seems to violate the model in which do_filter() makes sense.  
    if(!(row.parent())) return true;

    //First, substitute in variables
    for(size_t pos = 2; pos < fTypes.size(); ++pos) //Starting at column 2 because column 0 is a custom type that is related to picking and column 1 is whether an 
                                                    //object is visible.
    {
      if(copy.find("@"+std::to_string(pos)) != std::string::npos)
      {
        const auto val = get_col_value(pos, row);
        //std::cout << "substituting @" << pos << " with " << val << "\n";
        //TODO: Replace std::regex so I don't have to recompile it each time do_filter is called
        std::regex replace("(@"+std::to_string(pos)+")"); 
        copy = std::regex_replace(copy, replace, val); 
      }
    }
    //std::cout << "Expression after parameter substitution is:\n" << copy << "\n";

    try
    {
      //Next, evaluate expressions in parentheses
      size_t firstLeft = copy.find_first_of("(");
      while(firstLeft != std::string::npos)
      {
        //std::cout << "Back to main loop with expression " << copy << "\n";
        std::string suffix = copy.substr(firstLeft, std::string::npos);
        suffix = subexpr(suffix);
        copy.replace(firstLeft, std::string::npos, suffix);
        firstLeft = copy.find_first_of("(");
      }

      //Finally, evaluate the remaining expression of tokens
      return ev(copy);
    }
    catch(const util::GenException& e)
    {
      std::cerr << "Caught exception when processing formula " << get_text() << " in UserCut::do_filter(), so not doing anything.\n"
                << e.what() << "\n";
    }
    return true;
  }

  //TODO: This is horrible!  Is there a better semi-typesafe way to do this?  
  std::string UserCut::get_col_value(const int col, const Gtk::TreeRow& row)
  {
    //std::cout << "Entering get_col_value() for column " << col << "\n";
    //GType colType = fTypes[col];
    //auto typeChars = g_type_name(colType);
    std::string typeName = fTypes[col];
    /*if(typeChars != nullptr) typeName = std::string(typeChars); //VERY bad example
    else 
    {
      //throw util::GenException("Bad Type") << "Got nullptr for type name for column " << col << ".\n";
      std::cerr << "Got nullptr for type name for column " << col << ".\n";
      return "@"+std::to_string(col);
    }*/
    //std::cout << "Trying to read an object with GType " << typeName << "\n";
    //Dispatch to proper function if type is know.  If not, throw an exception. 
    //TODO: I could priobably write some TMP trickery to hide even this dispatch function...
    if(typeName == "gchararray") return get_from_col<gchararray>(col, row);
    if(typeName == "gdouble") return get_from_col<double>(col, row);
    if(typeName == "gboolean") return get_from_col<bool>(col, row);
    //TODO: The documentation at https://developer.gnome.org/glib/stable/glib-Basic-Types.html claims these exist...
    if(typeName == "gint") return get_from_col<gint>(col, row);
    if(typeName == "guint") return get_from_col<guint>(col, row);
    if(typeName == "gshort") return get_from_col<gshort>(col, row);
    if(typeName == "gushort") return get_from_col<gushort>(col, row);
    if(typeName == "gfloat") return get_from_col<gfloat>(col, row);
    /*throw util::GenException("Unrecognized Type") << "In mygl::UserCut::get_col_value(), got column of type " 
                                                  << typeName << " that is not currently handled.  If you feel very "
                                                  << "strongly about using this type, update this function.\n";*/
    std::cerr << "In mygl::UserCut::get_col_value(), got column of type " 
              << typeName << " that is not currently handled.  If you feel very "
              << "strongly about using this type, update this function.\n";
    return "@"+std::to_string(col);
  }

  std::string UserCut::subexpr(std::string expr)
  {
    //std::cout << "Expression " << expr << " was passed to subexpr().\n";
    size_t firstRight = expr.find_first_of(")");
    size_t firstLeft = expr.find_first_of("(", 1);
    while(firstLeft < firstRight && firstLeft != std::string::npos)
    {
      if(firstRight == std::string::npos)
      {
        throw util::GenException("User Cut") << "Got a string with mismatched parentheses in mygl::UserCut::subexpr().\n";
      }
      std::string suffix = subexpr(expr.substr(firstLeft, std::string::npos));
      expr.replace(firstLeft, std::string::npos, suffix);
      firstRight = expr.find_first_of(")");
      firstLeft = expr.find_first_of("(", 1);
    }
    const bool result = ev(expr.substr(1, firstRight-1)); //TODO: Is this the source of the small-size-string crash?
    expr.replace(0, firstRight+1, result?"true":"false");
    //std::cout << "After substituting " << std::boolalpha << result << ", expression is " << expr << "\n";
    return expr;
  }

  std::string UserCut::strip_spaces(std::string input)
  {
    //std::cout << "Removing spaces from string " << input << "\n";
    size_t nextSpace = input.find_first_of(" ");
    size_t nextNonSpace = input.find_first_not_of(" ");
    while(nextSpace != std::string::npos)
    {
      input.replace(nextSpace, nextNonSpace-nextSpace, "");
      nextSpace = input.find_first_of(" ");
      nextNonSpace = input.find_first_not_of(" ");
      //std::cout << "After an iteration of space removal from front, input is " << input << "\n";
    }
    return input;
  }


  bool UserCut::ev(std::string expr)
  {
    //std::cout << "Evaluating expression:\n" << expr << "\n";
  
    if(expr == "") return true; //Special cases
    if(expr == "true") return true;
    if(expr == "false") return false;
  
    const std::string comp = "<>=&|!";
    size_t firstComp = expr.find_first_of(comp);
    if(firstComp == std::string::npos)
    {
      throw util::GenException("Invalid Expression") << "Got expresssion " << expr << " with no comparison operators in mygl::UserCut::ev().\n";
    }

    size_t nextNonComp = expr.find_first_not_of(comp, firstComp+1);
    size_t nextComp = expr.find_first_of(comp, nextNonComp+1);
  
    //evaluate operators from left to right
    while(nextComp != std::string::npos)
    {
      //std::cout << "Entering tokenizing loop with expression " << expr << "\n";
      expr.replace(0, nextComp, ev(expr.substr(0, nextComp))?"true":"false");
      firstComp = expr.find_first_of(comp);
      nextNonComp = expr.find_first_not_of(comp, firstComp+1);
      nextComp = expr.find_first_of(comp, nextNonComp+1);
    }
    
    //Actually handle comparison operators
    const std::string lhs = strip_spaces(expr.substr(0, firstComp));
    const size_t lastComp = expr.find_last_of(comp);
    const std::string op = strip_spaces(expr.substr(firstComp, lastComp-firstComp+1));
    const std::string rhs = strip_spaces(expr.substr(lastComp+1, std::string::npos));
    //std::cout << "Going into operator table, lhs is " << lhs << ", rhs is " << rhs << ", and op is " << op << "\n";
  
    //TODO: Apparently, the empty string ("") is a number...
    const std::string numbers = "0123456789.";
    const bool lhsIsNum = (lhs.find_first_not_of(numbers) == std::string::npos);
    //if(!lhsIsNum) std::cout << "lhs is not a number.\n";
    const bool rhsIsNum = (rhs.find_first_not_of(numbers) == std::string::npos);
    //if(!rhsIsNum) std::cout << "rhs is not a number.\n";
  
    //Validate input
    if(lhsIsNum != rhsIsNum) 
    {
      throw util::GenException("User Cut") << "Cannot compare a number to a word.\n";
    }
    
    if(!lhsIsNum && op.find_first_of("<>") != std::string::npos)
    {
      throw util::GenException("User Cut") << "Operators <, >, <=, and >= only make sense when comparing numbers.\n";
    }
  
    //Handle operators
    //TODO: This could be made extendable for other binary operators by using a std::map here.
    if(op == "<")   
    {
      const auto lhsNum = std::stof(lhs);
      const auto rhsNum = std::stof(rhs);
      return lhsNum < rhsNum;
    }
    if(op == ">")
    {
      const auto lhsNum = std::stof(lhs);
      const auto rhsNum = std::stof(rhs);
      return lhsNum > rhsNum;
    }
    if(op == "<=")
    {
      const auto lhsNum = std::stof(lhs);
      const auto rhsNum = std::stof(rhs);
      return lhsNum <= rhsNum;
    }
    if(op == ">=")
    {
      const auto lhsNum = std::stof(lhs);
      const auto rhsNum = std::stof(rhs);
      return lhsNum >= rhsNum;
    }
    if(op == "==")
    {
      return lhs == rhs;
    }
    if(op == "!=")
    {
      return lhs != rhs;
    }

    //Boolean operators
    bool lhsBool;
    if(lhs == "true") lhsBool = true;
    else if(lhs == "false") lhsBool = false;
    else 
    {
      throw util::GenException("User Cut") << "Cannot convert lhs string " << lhs 
                                           << " to a boolean, and boolean operators can only be used with boolean values.\n";
    }
    bool rhsBool;
    if(rhs == "true") rhsBool = true;
    else if(rhs == "false") rhsBool = false;
    else
    {
      throw util::GenException("User Cut") << "Cannot convert rhs string " << rhs << " to a boolean, and boolean "
                                           << "operators can only be used with boolean values.\n";
    }
    if(op == "&&") return lhsBool && rhsBool;
    if(op == "||") return lhsBool || rhsBool;

    throw util::GenException("User Cut") << "Got to end of mygl::UserCut::ev(), so operator " << op << " is probably not supported.\n";
    return true;
  }  
}
