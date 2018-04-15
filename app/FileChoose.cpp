//File: FileChoose.cpp
//Brief: An imgui and ROOT-powered tool for choosing a file from a working directory provided by the user.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//The core ImGUI function definitions
#include "imgui.h"

//local includes
#include "FileChoose.h"

//ROOT includes
#include "TSystemFile.h"
#include "TSystem.h"
#include "TList.h"

//TODO: Remove me
#include <iostream>

namespace file
{
  FileChoose::FileChoose(): fPaths(), fCurrent(nullptr), fBuffer()
  {
    //Initialize with pwd
    std::string pwd(gSystem->pwd());

    fPaths.emplace(std::make_pair(pwd, new TSystemDirectory(pwd.substr(pwd.find_last_of('/')+1, std::string::npos).c_str(), pwd.c_str())));
    fCurrent = fPaths.begin()->second;

    std::copy(pwd.data(), pwd.data()+pwd.size(), fBuffer.data());
  }

  TSystemFile* FileChoose::Render(const std::string& extension)
  {
    //Address bar 
    std::string pwd(fCurrent->GetName());
    //std::copy(pwd.data(), pwd.data()+pwd.size(), fBuffer.data());
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
    auto chosen = DrawFile(fCurrent, extension);
    ImGui::NextColumn();

    ImGui::Columns(1);
    return chosen;
  }

  TSystemFile* FileChoose::DrawFile(TSystemFile* file, const std::string& extension)
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
            auto chosen = DrawFile((TSystemFile*)obj, extension);
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
      if(baseName.substr(baseName.find_first_of('.'), std::string::npos) == extension)
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

  void FileChoose::AddPath(const std::string& name)
  {    
    const auto found = name.find_last_of('/');
    if(found) fPaths[name.substr(found+1, std::string::npos)] = new TSystemDirectory(name.substr(found+1, std::string::npos).c_str(), name.c_str());
    else fPaths[name] = new TSystemDirectory(name.c_str(), name.c_str());
  }
} 
