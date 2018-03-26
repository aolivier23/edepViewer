//File: FileChoose.h
//Brief: Tool for choosing a file from an absolute path or a relative path from a user-provided abolute path.
//       Powered by ROOT's TSystemFile and TSystem for now. 
//Author: Andrew Olivier aolivier@ur.rochester.edu

//ROOT includes
#include "TSystemDirectory.h"

//c++ includes
#include <memory>
#include <map>
#include <array>

class TSystemFile;

namespace file
{
  class FileChoose
  {
    public:
      FileChoose();
      virtual ~FileChoose() = default;

      TSystemFile* Render(const std::string& extension); //Returns a nullptr if a file has not yet been chosen

      void AddPath(const std::string& path); //Tell a fileChoose about a file system path for the user to see

    protected:
      std::map<std::string, TSystemDirectory*> fPaths; //List of absolute paths this FileChoose knows about
      TSystemDirectory* fCurrent; //Current path that the user is exploring
      std::array<char, 512> fBuffer; //Buffer for user path input

      TSystemFile* DrawFile(TSystemFile* file);
  };
}
