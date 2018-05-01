//File: Factory.cpp
//Brief: A Factory keeps track of all of the plugins of a given type (parameter).  
//       To do this, a Factory must be a singleton, and plugins invoke a macro at 
//       compile time to tell the register about themselves. 
//Author: Andrew Olivier aolivier@ur.rochester.edu
//Inspired by the conversation at https://codereview.stackexchange.com/questions/119812/compile-time-plugin-system

//yaml-cpp include for configuration
#include "yaml-cpp/yaml.h"

//c++ includes
#include <memory> //For std::unique_ptr
#include <map>
//TODO: Remove me
#include <iostream>

#ifndef PLGN_FACTORY_CPP
#define PLGN_FACTORY_CPP

namespace plgn
{
  template <class BASE>
  class RegistrarBase
  {
    public:
      virtual ~RegistrarBase() = default;

      virtual std::unique_ptr<BASE> NewPlugin(const YAML::Node& config) = 0;
  };

  template <class BASE>
  class Factory
  {
    public:
      Factory(Factory&) = delete;
      Factory(Factory&&) = delete;
      Factory& operator =(Factory&) = delete;

      static Factory& instance()
      {
        static Factory reg;
        return reg;
      }

      virtual ~Factory() = default;
 
      void Add(const std::string& name, RegistrarBase<BASE>* reg)
      {
        fNameToReg[name] = reg;
      }

      //TODO: Turn string into parameter system interface
      std::unique_ptr<BASE> Get(const YAML::Node& config)
      {
        if(config.IsNull()) return std::unique_ptr<BASE>(); //Return invalid unique_ptr
        const auto found = fNameToReg.find(config.Tag());
        if(found != fNameToReg.end()) return found->second->NewPlugin(config); 
        return std::unique_ptr<BASE>(); //Return invalid unique_ptr
      }

    private:
      Factory(): fNameToReg() {}

      std::map<std::string, RegistrarBase<BASE>*> fNameToReg;
  };

  //According to the Stack Overflow article where I learned this technique, interposing 
  //a registrar class between DERIVED and Factory makes sure that expensive plugins don't 
  //have to be instantiated if they are not used.  This is one of the main advantages I want 
  //from my plugin system, and it allows me to create a plugin from a configuration 
  //object when I need it.  
  template <class BASE, class DERIVED>
  class Registrar: public RegistrarBase<BASE>
  {
    public:
      Registrar(const std::string& name)
      {
        auto& reg = Factory<BASE>::instance();
        reg.Add(name, this);
        std::cout << "Registered a plugin named " << name << "\n";
      }

      virtual ~Registrar() = default;

      virtual std::unique_ptr<BASE> NewPlugin(const YAML::Node& config)
      {
        return std::unique_ptr<BASE>(new DERIVED(config));
      }
  };
}

#define REGISTER_PLUGIN(CLASSNAME, BASE) \
namespace \
{ \
  static plgn::Registrar<BASE, CLASSNAME> CLASSNAME_reg(#CLASSNAME); \
}

#endif //PLGN_FACTORY_CPP 
