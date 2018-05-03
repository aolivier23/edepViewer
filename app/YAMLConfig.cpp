//File: YAMLConfig.cpp
//Brief: Interprets a YAML::Node (from yaml-cpp) as a tree of other YAML::Nodes.  Node names cannot be changed, but mappings' values can, and 
//       Nodes can be added or removed.  Can save the final YAML configuration to a file.  
//Author: Andrew Olivier aolivier@ur.rochester.edu

#include "app/YAMLConfig.h"

//imgui include
#include "imgui.h"

namespace gui
{
  bool YAMLConfig::Render()
  {
    const bool oldShow = fShowGUI;
    if(fShowGUI)
    {
      ImGui::Begin("Configuration", &fShowGUI);
      Render(fConfig);
      ImGui::End();
    } 
    return oldShow != fShowGUI; 
  }

  void YAMLConfig::Render(YAML::Node node) //YAML::Node::iterator::operator *() returns a YAML::Node rather than a YAML::Node& !?
  {
    //Render the YAML::Node
    if(node.IsSequence())
    {
      //TODO: Controls to remove this Node or add a child
      for(auto child =  node.begin(); child != node.end(); ++child) 
      {
        ImGui::Bullet(); //Also lines InputText() up with TreeNode
        Render(*child);
      }
    }
    if(node.IsMap())
    {
      for(auto iter = node.begin(); iter != node.end(); ++iter)
      {
        if(ImGui::TreeNode(iter->first.as<std::string>().c_str())) 
        {
          Render(iter->second); 
          ImGui::TreePop();
        }
      }
    }
    if(node.IsScalar())
    {
      //TODO: Float/Int entry if appropriate?  Can yaml-cpp help me detect this situation?
      std::array<char, 100> buf({'\0'}); //TODO: This places a maximum size on the text that can be entered into a scalar Node
      const auto value = node.as<std::string>();
      for(size_t pos = 0; pos < value.size() && pos < buf.size(); ++pos) buf[pos] = value[pos]; //TODO: Better way to set up buffer?
      ImGui::SameLine();
      if(ImGui::InputText(("##"+value).c_str(), buf.data(), buf.size(), ImGuiInputTextFlags_EnterReturnsTrue))
      {
        node = std::string(buf.data());
      }
    }
  }
}
