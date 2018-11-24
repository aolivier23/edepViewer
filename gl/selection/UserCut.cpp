//File: UserCut.cpp
//Brief: A Gtk::Entry that provides a do_filter() function for Gtk::TreeModelFilter to 
//       filter entries based on user input.  See header for documentation of cut syntax.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//gl includes
#include "UserCut.h"
#include "util/GenException.h"
#include "gl/metadata/Row.h"

//c++ includes
#include <string>
#include <iostream>
#include <regex>

namespace mygl
{
  UserCut::UserCut(const size_t nCols): fInput({'t', 'r', 'u', 'e'}), fNCols(nCols)
  {   
  }

  bool UserCut::do_filter(const ctrl::Row& row) 
  {
    std::string copy(fInput.data());

    //First, substitute in variables
    for(size_t pos = 0; pos < fNCols; ++pos) //Starting at column 2 because column 0 is a custom type that is related to picking and column 1 is whether an 
                                                    //object is visible.
    {
      if(copy.find("@"+std::to_string(pos)) != std::string::npos)
      {
        const auto val = row[pos];
        //std::cout << "substituting @" << pos << " with " << val << "\n";
        //TODO: Replace std::regex so I don't have to recompile it each time do_filter is called
        std::regex replace("(@"+std::to_string(pos)+")"); 
        copy = std::regex_replace(copy, replace, val); 
      }
    }
    //std::cout << "Expression after parameter substitution is:\n" << copy << "\n";

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
    //TODO: e- is not a number!
    const std::string numbers = "0123456789"; //"e" for scientific notation
    const std::string symbols = ".-e";
    const bool lhsIsNum = (lhs.find_first_not_of(numbers+symbols) == std::string::npos) && (lhs.find_first_of(numbers) != std::string::npos);
    //if(!lhsIsNum) std::cout << "lhs is not a number.\n";
    const bool rhsIsNum = (rhs.find_first_not_of(numbers+symbols) == std::string::npos) && (rhs.find_first_of(numbers) != std::string::npos);
    //if(!rhsIsNum) std::cout << "rhs is not a number.\n";
  
    //Validate input
    if(lhsIsNum != rhsIsNum) 
    {
      throw util::GenException("User Cut") << "Cannot compare a number to a word.  lhs is " << lhs << ", and rhs is " << rhs << ".\n";
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
      throw util::GenException("User Cut") << "/annot convert lhs string " << lhs 
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
