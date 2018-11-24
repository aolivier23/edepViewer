//File: ParseTest.cpp
//Brief: Parses the user's logic expression, substituting variables, and returns true if the expression is 
//       true and false otherwise.  Test for user-defined cuts in edepsim event display.  Should accept 
//       the following syntax:
//
//Syntax for "user code" that is typed into the Gtk::Entry:
//       @<number>: Value in column <number>
//       lhs < rhs: True if lhs is less than rhs.  False otherwise.  
//       lhs > rhs: True if lhs is greater than rhs.  False otherwise. 
//       lhs == rhs: True is lhs is equal to rhs.  False otherwise.
//       lhs != rhs: True if lhs is not equal to rhs.  False otherwise.
//       lhs <= rhs: True if lhs is less than or equal to rhs.  False otherwise.
//       lhs >= rhs: True is lhs is greater than or equal to rhs.  False otherwise.
//       first && second: True if first is true and second is true.  False otherwise.
//       first || second: True if first is true or if second is true.  False otherwise.  
//       first && (second || third): True if first is true and:
//                                   second is true
//                                   OR
//                                   third is true
//       first && second || third: True if first and second are true OR third is true
//Author: Andrew Olivier aolivier@ur.rochester.edu

//c++ includes
#include <regex>
#include <iostream>
#include <string>

std::string stripSpaces(std::string input)
{
  std::cout << "Removing spaces from string " << input << "\n";
  size_t nextSpace = input.find_first_of(" ");
  size_t nextNonSpace = input.find_first_not_of(" ");
  while(nextSpace != std::string::npos)
  {
    input.replace(nextSpace, nextNonSpace-nextSpace, "");
    nextSpace = input.find_first_of(" ");
    nextNonSpace = input.find_first_not_of(" ");
    std::cout << "After an iteration of space removal from front, input is " << input << "\n";
  }
  return input;
}

bool ev(std::string expr)
{
  std::cout << "Evaluating expression:\n" << expr << "\n";

  if(expr == "") return true; //Special case

  const std::string comp = "<>=&|!";
  size_t firstComp = expr.find_first_of(comp);
  size_t nextNonComp = expr.find_first_not_of(comp, firstComp+1);
  size_t nextComp = expr.find_first_of(comp, nextNonComp+1);

  //evaluate operators from left to right
  while(nextComp != std::string::npos)
  {
    std::cout << "Entering tokenizing loop with expression " << expr << "\n";
    expr.replace(0, nextComp, ev(expr.substr(0, nextComp))?"true":"false");
    firstComp = expr.find_first_of(comp);
    nextNonComp = expr.find_first_not_of(comp, firstComp+1);
    nextComp = expr.find_first_of(comp, nextNonComp+1);
  }
  
  //Actually handle comparison operators
  const std::string lhs = stripSpaces(expr.substr(0, firstComp));
  const size_t lastComp = expr.find_last_of(comp);
  const std::string op = stripSpaces(expr.substr(firstComp, lastComp-firstComp+1));
  const std::string rhs = stripSpaces(expr.substr(lastComp+1, std::string::npos));
  std::cout << "Going into operator table, lhs is " << lhs << ", rhs is " << rhs << ", and op is " << op << "\n";

  const std::string numbers = "0123456789.";
  const bool lhsIsNum = (lhs.find_first_not_of(numbers) == std::string::npos);
  if(!lhsIsNum) std::cout << "lhs is not a number.\n";
  const bool rhsIsNum = (rhs.find_first_not_of(numbers) == std::string::npos);
  if(!rhsIsNum) std::cout << "rhs is not a number.\n";

  //Validate input
  if(lhsIsNum != rhsIsNum) 
  {
    std::cerr << "Cannot compare a number to a word.\n";
    //TODO: throw exception
  }
  
  if(!lhsIsNum && op.find_first_of("<>") != std::string::npos)
  {
    std::cerr << "Operators <, >, <=, and >= only make sense when comparing numbers.\n";
    //TODO: throw exception
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
    std::cerr << "Cannot convert lhs string " << lhs << " to a boolean, and boolean operators can only be used with boolean values.\n";
    //TODO: Throw exception
  }
  bool rhsBool;
  if(rhs == "true") rhsBool = true;
  else if(rhs == "false") rhsBool = false;
  else
  {
    std::cerr << "Cannot convert rhs string " << rhs << " to a boolean, and boolean operators can only be used with boolean values.\n";
    //TODO: Throw exception
  }
  if(op == "&&") return lhsBool && rhsBool;
  if(op == "||") return lhsBool || rhsBool;
 
  std::cerr << "Got to end of ev() without finding a match for operator " << op << ", so this operator must not be supported.\n";
  //TODO: Throw exception
  return true; 
}

//Evaluate parentheses-enclosed subexpressions.  
std::string subexpr(std::string expr)
{
  std::cout << "Expression " << expr << " was passed to subexpr().\n";
  size_t firstRight = expr.find_first_of(")");
  size_t firstLeft = expr.find_first_of("(", 1);
  while(firstLeft < firstRight) 
  {
    if(firstRight == std::string::npos)
    {
      std::cerr << "Mismatched parentheses.\n"; 
      //TODO: throw exception
    }
    std::string suffix = subexpr(expr.substr(firstLeft, std::string::npos));
    expr.replace(firstLeft, std::string::npos, suffix); 
    firstRight = expr.find_first_of(")");
    firstLeft = expr.find_first_of("(", 1);
  }
  const bool result = ev(expr.substr(1, firstRight-1));
  expr.replace(0, firstRight+1, result?"true":"false");
  std::cout << "After substituting " << std::boolalpha << result << ", expression is " << expr << "\n";
  return expr;
}

bool ParseTest(const std::string& input, std::initializer_list<std::string>&& userParams)
{
  std::cout << "The number of paramters is " << userParams.size() << "\n";
  std::string copy = input;

  //First, substitute in variables
  std::vector<std::string> params(userParams.begin(), userParams.end());
  for(size_t pos = 0; pos < params.size(); ++pos)
  {
    std::cout << "substituting @" << pos << " with " << params[pos] << "\n";
    std::regex replace("(@"+std::to_string(pos)+")");
    copy = std::regex_replace(copy, replace, params[pos]);
  }
  std::cout << "Expression after parameter substitution is:\n" << copy << "\n";

  //Next, evaluate expressions in parentheses
  size_t firstLeft = copy.find_first_of("(");
  while(firstLeft != std::string::npos)
  {
    std::cout << "Back to main loop with expression " << copy << "\n";
    std::string suffix = copy.substr(firstLeft, std::string::npos);
    suffix = subexpr(suffix);
    copy.replace(firstLeft, std::string::npos, suffix);
    firstLeft = copy.find_first_of("(");
  }

  //Finally, evaluate the remaining expression of tokens
  return ev(copy);
}
