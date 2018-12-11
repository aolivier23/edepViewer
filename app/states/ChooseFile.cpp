//File: ChooseFile.cpp
//Brief: A ChooseFile is a State that pops up a GUI for the user to choose the next file to 
//       view.  It doesn't care about what event processing is happening, but it doesn't 
//       start any more event processing in anticipation of an upcoming NewFile event.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef FSM_CHOOSEFILE_CPP
#define FSM_CHOOSEFILE_CPP

//Local includes
#include "State.h"

//ROOT includes
#include "TSystemDirectory.h"
#include "TSystem.h"

//The core ImGUI function definitions
#include "imgui.h"

//c++ includes
#include <map>
#include <iostream> //TODO: Remove me by replacing with windows

namespace fsm
{
  template <class TO> //TO is a State that FileChoose will transition to when the user chooses a file.  
                      //TO::TO() takes a std::string as argument
  class ChooseFile: public State
  {
    public:
      ChooseFile(const std::string& extension): fExtension(extension) {}
      virtual ~ChooseFile() = default;

    protected:
      virtual std::unique_ptr<State> doPoll(evd::Window& /*window*/) override
      {
        //Address bar 
        std::string pwd(fCurrent->GetName());
        ImGui::InputText("##Path", fBuffer.data(), fBuffer.size());
        ImGui::SameLine();
        if(ImGui::Button("Go")) 
        {
          std::string bufName(fBuffer.data());

          //TSystemDirectory can't actually validate the directories I create this way.  So, use TSystem calls 
          //to find out whether the user-selected directory exists.
          auto dir = gSystem->OpenDirectory(bufName.c_str());
          if(dir)
          {
            const auto slash = bufName.find_last_of('/');
            TSystemDirectory* dir;
            if(slash != std::string::npos) dir = new TSystemDirectory(bufName.substr(slash+1, std::string::npos).c_str(), bufName.c_str());
            else dir = new TSystemDirectory(bufName.c_str(), bufName.c_str());
            fCurrent = dir;
            AddPath(dir->GetTitle());
          }
          else
          {
            std::cerr << fBuffer.data() << ": No such file or directory.\n";
            std::copy(pwd.data(), pwd.data()+pwd.size(), fBuffer.data());
          }
        }

        ImGui::Separator();
        ImGui::Columns(2);

        //List of current "root" directories known
        for(const auto& pair: fPaths)
        {
          auto ptr = pair.second;
          std::string name(ptr->GetTitle());
          if(ImGui::Button(ptr->GetTitle())) fCurrent = ptr;
        }
        ImGui::NextColumn();

        //Tree view of current directory
        auto chosen = DrawFile(fCurrent);
        ImGui::NextColumn();
        ImGui::Columns(1);

        if(chosen)
        {
          std::string name(chosen->GetTitle());
          name += "/";
          name += chosen->GetName();
          return std::unique_ptr<State>(new TO(name));
        }
      
        return nullptr;
      }


    private:
      //State for file choosing GUI
      const std::string fExtension; //Only show files with this extension
      std::map<std::string, TSystemDirectory*> fPaths; //List of absolute paths this FileChoose knows about
      TSystemDirectory* fCurrent; //Current path that the user is exploring
      std::array<char, 512> fBuffer; //Buffer for user path input

      TSystemFile* DrawFile(TSystemFile* file) const
      {
        std::string name(file->GetName());
        const auto found = name.find_last_of('/');
        std::string baseName;
        if(found != std::string::npos) baseName = name.substr(found+1, std::string::npos);
        else baseName = name;

        auto dir = dynamic_cast<TSystemDirectory*>(file);
        if(dir) //If a directory, decide whether to expand to children
        {
          if(ImGui::TreeNode((baseName+"##dir").c_str()))
          {
            auto files = dir->GetListOfFiles();
            if(files)
            {
              for(auto obj: *files)
              {
                auto chosen = DrawFile((TSystemFile*)obj);
                if(chosen)
                {
                  ImGui::TreePop();
                  return chosen;
                }
              }
            }
            ImGui::TreePop();
          }
        }
        else
        {
          if(baseName.substr(baseName.find_first_of('.'), std::string::npos) == fExtension)
          {
            ImGui::Indent(ImGui::GetTreeNodeToLabelSpacing()); 
            if(ImGui::Button((baseName+"##File").c_str())) //Otherwise, create a button to choose this file
            {
              ImGui::Unindent(ImGui::GetTreeNodeToLabelSpacing());
              return file;
            }
            ImGui::Unindent(ImGui::GetTreeNodeToLabelSpacing());
          }
        }
        return nullptr;
      }

      void AddPath(const std::string& name)
      {
        const auto found = name.find_last_of('/');
        if(found) 
        {
          fPaths[name.substr(found+1, std::string::npos)] = new TSystemDirectory(name.substr(found+1, std::string::npos).c_str(), 
                                                                                 name.c_str());
        }
        else fPaths[name] = new TSystemDirectory(name.c_str(), name.c_str());
      }
  };
}

#endif //FSM_CHOOSEFILE_CPP
