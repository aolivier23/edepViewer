//File: Scene.cpp
//Brief: A Scene collects the resources to draw a group of related objects using opengl.   
//Author: Andrew Olivier aolivier@ur.rochester.edu

//imgui includes
#include "imgui.h"

//gl includes
#include "gl/Scene.h"
#include "gl/model/Drawable.h"
#include "gl/SceneConfig.cpp"

//c++ includes
#include <sstream>
#include <array>
#include <algorithm>

namespace mygl
{
  //Warning: Make sure that Gtk::GLArea::make_current() is called just before this function so that 
  //         the proper GL context is bound for shader allocation.
  Scene::Scene(const std::string& name, const std::string& fragSrc, const std::string& vertSrc, std::shared_ptr<mygl::ColRecord>& cols, 
               std::unique_ptr<SceneConfig>&& config): 
                                                      fName(name), fActive(), fHidden(), fShader(fragSrc, vertSrc), 
                                                      fSelectionShader(INSTALL_GLSL_DIR "/selection.frag", vertSrc),
                                                      fSelectPath(), fModel(cols), fSelfCol(cols->fDrawSelf), fIDCol(cols->fVisID), 
                                                      fCols(cols), fCutBar(cols->size()), fBuffer(fCutBar.fInput), fVAO(new VAO()), 
                                                      fConfig(std::move(config))
  {
  }

  Scene::Scene(const std::string& name, const std::string& fragSrc, const std::string& vertSrc, const std::string& geomSrc, 
               std::shared_ptr<mygl::ColRecord>& cols, std::unique_ptr<SceneConfig>&& config): fName(name), fActive(), fHidden(), 
               fShader(fragSrc, vertSrc, geomSrc), fSelectionShader(INSTALL_GLSL_DIR "/selection.frag", vertSrc, geomSrc), fModel(cols), 
               fSelfCol(cols->fDrawSelf), fIDCol(cols->fVisID), fCols(cols), fCutBar(cols->size()), fBuffer(fCutBar.fInput), 
               fVAO(new VAO()), fConfig(std::move(config)), fSelectPath()
  {
  }

  Scene::~Scene() {}

  TreeModel::iterator Scene::NewTopLevelNode()
  {
    return fModel.NewNode();
  }

  void Scene::apply_filter(const TreeModel::iterator& parent)
  {
    std::list<TreeModel::iterator> toMove; //List of children to move to end of parent

    for(auto child = parent->begin(); child != parent->end(); ++child)
    {
      if(!fCutBar.do_filter(child))
      {
        auto& visible = (*child)[fSelfCol];
        if(visible)
        {
          visible = false;
          this->Transfer(fActive, fHidden, (*child)[fIDCol]);
        }
        toMove.push_back(child);
      }
      apply_filter(child);
    }

    //Move things around only after the loop
    for(auto& iter: toMove) parent->MoveToEnd(iter);
  }

  //Call this before Render() to get updates from user interaction with list tree.  
  void Scene::RenderGUI()
  {
    //Cut bar
    if(ImGui::InputText("##Cut", fBuffer.data(), fBuffer.size(), ImGuiInputTextFlags_EnterReturnsTrue)) 
    //TODO: Help text about filter syntax
    {
      fCutBar.fInput = fBuffer;

      //Turn off drawing for 3D objects whose metdata don't pass cut
      //TODO: Combine metadata and object rendering to remove the need for this 
      //      convoluted filter system.  Could probably remove fActive and fHidden as well 
      //      and simplify Scene considerably.  This is outside the scope of what I am 
      //      working on right now, but it is a great future project if 
      //      imgui works out.  
      try
      {
        for(auto top = fModel.begin(); top != fModel.end(); ++top) apply_filter(top);
      }
      catch(const util::GenException& e)
      {
        std::cerr << "Caught exception during formula processing:\n" << e.what() << "\nIgnoring cuts for this Scene.\n";
      }
    }

    //Tree column labels
    //Calculate the total length of text I will want to display
    float width = 5.*ImGui::GetTreeNodeToLabelSpacing(); //Make space for 5 tree node opens
    for(size_t col = 2; col < fCols->size(); ++col)
    {
      width += 2.*ImGui::CalcTextSize(fCols->Name(col).c_str()).x + ImGui::GetStyle().ItemSpacing.x;
    }

    ImGui::SetNextWindowContentSize(ImVec2(width, 0)); //Make space for 5 tree node opens plus column labels
    ImGui::BeginChild("List Tree", ImVec2(0,0), true, ImGuiWindowFlags_HorizontalScrollbar);
    ImGui::Columns(fCols->size()-1);
    ImGui::Text(fCols->Name(1).c_str());
    ImGui::NextColumn();
    for(size_t col = 2; col < fCols->size(); ++col) 
    {
      ImGui::Text(fCols->Name(col).c_str());
      ImGui::NextColumn();
    }
    ImGui::Separator();

    try
    {
      for(auto iter = fModel.begin(); iter != fModel.end(); ++iter) 
      {
        DrawNode(iter, true, 1, fSelectPath.begin()); 
      }
    }
    catch(const util::GenException& e)
    {
      std::cerr << "Caught std::exception " << e.what() << " when parsing formula.  Ignoring filter expression.\n";
    }
    ImGui::Columns(1); //Leave the multi-column context?
    ImGui::EndChild();
  }

  void Scene::Render(const glm::mat4& view, const glm::mat4& persp) 
  {
    //Note that these uniform names assume that like-named uniforms are 
    //handled by the shader programs used to form fShader
    fConfig->BeforeRender();
    fVAO->Use();
    fShader.Use();
    fShader.SetUniform("view", view);
    fShader.SetUniform("projection", persp);
    fShader.SetUniform("model", glm::mat4());  //In case Drawables don't set their own model matrices.  Setting the 
                                               //same uniform twice shouldn't be a problem, right?
    for(auto& pair: fActive) pair.second->Draw(fShader); //This is different from my old DRAWER contract because 
                                                         //Drawables are now responsible for binding their own 
                                                         //model matrices.
    fConfig->AfterRender();

    glBindVertexArray(0);
  }

  void Scene::RenderSelection(const glm::mat4& view, const glm::mat4& persp)
  {
    //Note that these uniform names assume that like-named uniforms are 
    //handled by the shader programs used to form fShader
    fVAO->Use();
    fSelectionShader.Use();
    fSelectionShader.SetUniform("view", view);
    fSelectionShader.SetUniform("projection", persp);
    fSelectionShader.SetUniform("model", glm::mat4());  //In case Drawables don't set their own model matrices.  Setting the 
                                                        //same uniform twice shouldn't be a problem, right?
    for(auto& pair: fActive) 
    {
      fSelectionShader.SetUniform("idColor", pair.first); //Each VisID is a unique color that can be drawn by opengl.  
                                                          //So, draw this object with that color so that its' color 
                                                          //can be mapped back to its' VisID if the user clicks on it.
      pair.second->Draw(fSelectionShader); //This is different from my old DRAWER contract because 
                                           //Drawables are now responsible for binding their own 
                                           //model matrices.
    }

    glBindVertexArray(0);
  }

  //This function destroys Drawables that might (and probably should) make opengl calls in their destructors.  
  //Make sure that the openGL context for this Scene is current when this is called.  
  void Scene::RemoveAll() //Might be useful when updating event
  {
    fActive.erase(fActive.begin(), fActive.end());
    fHidden.erase(fHidden.begin(), fHidden.end());
    fVAO.reset(new VAO());
    fModel.Clear();
    fSelectPath.clear();
  }

  //All of the magic happens here.  Moving a unique_ptr from one container to another seems like a very difficult task... 
  void Scene::Transfer(std::map<VisID, std::unique_ptr<Drawable>>& from, std::map<VisID, std::unique_ptr<Drawable>>& to, const VisID& id)
  {
    const auto found = from.find(id);
    if(found == from.end())
    {
      std::stringstream str;
      str << "from:\n";
      for(auto& pair: from) str << pair.first << "\n";
      str << "to:\n";
      for(auto& pair: to) str << pair.first << "\n";
      throw util::GenException("ID Does Not Exist") << "In view::Scene::Transfer(), tried to move "
                                                    << "a drawable with ID " << id << " in scene " << fName
                                                    << ", but there is no such object.  The IDs currently "
                                                    << "registered are:\n" << str.str() << "\n";
    }
    //Very manual memory management, but it (hopefully) works...
    auto copyPtr = found->second.release(); //Aha!
    from.erase(found);
    to.emplace(id, std::unique_ptr<Drawable>(copyPtr)).second;
  }

  bool Scene::FindID(const mygl::VisID& id, const mygl::TreeModel::iterator iter)
  {
    //If this Node has been selected
    const auto& localID = (*iter)[this->fIDCol];
    if(localID == id) 
    {
      fSelectPath.insert(fSelectPath.begin(), localID);
      return true;
    }

    //If a child of this Node has been selected
    for(auto child = iter->begin(); child != iter->end(); ++child)
    {
      if(FindID(id, child))
      {
        fSelectPath.insert(fSelectPath.begin(), localID);
        return true;
      }
    }
    
    //This Node is not in the path to the selected Node
    return false;
  }

  //TODO: Tell other Scenes that this VisID has been selected
  bool Scene::SelectID(const mygl::VisID& id)
  {
    std::cout << "Trying to select object with VisID " << id << "\n";

    //Regardless of what was selected, unselect the last thing that was selected
    const auto prevID = fSelectPath.empty()?mygl::VisID():fSelectPath.back();
    auto prev = fActive.find(prevID); //The previously selected object might have been disabled since 
                                      //it was selected.
    //TODO: Make border color and width a user parameter
    if(prev != fActive.end()) prev->second->SetBorder(0., glm::vec4(1., 0., 0., 1.));
    else
    {
      prev = fHidden.find(prevID);
      if(prev != fHidden.end()) prev->second->SetBorder(0., glm::vec4(1., 0., 0., 1.));
      else std::cerr << "Failed to find previously selected object with ID " << prevID << "\n";
    }

    fSelectPath.clear();

    //Find all objects with VisID id in the TreeModel.  Needed for generating a tooltip.
    bool result = false;
    for(auto child = fModel.begin(); child != fModel.end(); ++child)
    {
      if(FindID(id, child)) //FindID constructs the current selection path if it returns true
      {
        result = true;
        break;
      }
    }

    if(result)
    {
      std::cout << "Highlighting 3D object with ID " << id << "\n";
      //Highlight graphics object that was selected
      auto found = fActive.find(id);
      if(found != fActive.end())
      {
        found->second->SetBorder(0.01, glm::vec4(1., 0., 0., 1.));
        std::cout << "Selected the following path of objects:\n";
        for(const auto& id: fSelectPath) std::cout << id << "\n";

        return true;
      }
    }

    return false;
  }

  //Draw a Node that could have selected children
  void Scene::DrawNode(const mygl::TreeModel::iterator iter, const bool top, const size_t depth, std::vector<mygl::VisID>::iterator selectSearch)
  {
    const bool selected = (selectSearch != fSelectPath.end() && selectSearch < fSelectPath.end()-1 && ((*iter)[fIDCol] == *selectSearch));
    if(selected) ImGui::SetNextTreeNodeOpen(true); 
    const bool open = DrawNodeData(iter, top); //TODO: Figure out whether a TreeNode() is open without drawing it?  Default to open TreeNode()s?
  
    if(open)
    {
      //Draw children of this Node while looking for selected Node
      //If too many children, use ListClipper
      //TODO: If I use ListClipper, I seem to get the wrong cursor position for children.  I think I get back the right cursor position for 
      //      children if I comment out the SetCursorPosYAndSetupDummyPrevLine() call in ImGuiListClipper::End().  It seems like I could set 
      //      items_count to INT_MAX to get the same behavior, but I cannot scroll when I do that.  So, I might want to consider suggesting a 
      //      bug fix/feature in ImGui or writing a custom TreeClipper that does most of what ImGuiListClipper does.  
      /*if(iter->NChildren() > 200)
      {
        ImGuiListClipper clipper(iter->NChildren(), ImGui::GetTextLineHeightWithSpacing());
        while(clipper.Step())
        {
          const size_t start = clipper.DisplayStart;
          const size_t end = clipper.DisplayEnd;
          size_t pos = 0;
          auto child = iter->begin();
          for(; child != iter->end() && pos < end; ++child)
          {
            if(pos >= start) 
            {
              if(selected) DrawNode(child, false, selectSearch+1);
              else DrawNode(child, false);
            }
            ++pos;
          }
          
          //ListClipper doesn't handle the last entry in a list very well, so draw it myself
          //if(selected) DrawNode(child, false, selectSearch+1);
          //else DrawNode(child, false);
        }
      }
      else*/
      {
        //Make sure column is wide enough for checkboxes
        if(open && ImGui::GetColumnWidth() < ImGui::GetTreeNodeToLabelSpacing()*(depth+1))
        {
          ImGui::SetColumnWidth(-1, ImGui::GetTreeNodeToLabelSpacing()*(depth+1)); //Leave extra space for the checkbox
        }

        for(auto child = iter->begin(); child != iter->end(); ++child)
        {
          if(selected) DrawNode(child, false, depth+1, selectSearch+1);
          else DrawNode(child, false, depth+1);
        }
      }
      ImGui::TreePop();
    }
  }

  bool Scene::DrawNodeData(const mygl::TreeModel::iterator iter, const bool top)
  {
    //This tree entry needs a unique ID for imgui.  Turn the 
    //VisID it includes into a string since I am not currently
    //putting the same VisID in more than one place in a given tree.
    std::stringstream ss;
    const auto& id = (*iter)[fIDCol];
    ss << id;
    const auto idBase = ss.str();

    //If this Node's VisID is selected, highlight this line in the list tree
    const bool selected = (!fSelectPath.empty()) && (id == fSelectPath.back()); 
    bool open = false;
    ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;    
    if(iter->NChildren() == 0) node_flags |= ImGuiTreeNodeFlags_Leaf;
    open = ImGui::TreeNodeEx(("##Node"+idBase).c_str(), node_flags);

    //Draw this Node's data
    //Draw a checkbox for the first column
    ImGui::SameLine(); //Put the checkbox on the same line as the tree arrow
    if(!top)
    {
      if(ImGui::Checkbox(("##Check"+idBase).c_str(), &(*iter)[fSelfCol]))
      {
        if((*iter)[fSelfCol]) Transfer(fHidden, fActive, (*iter)[fIDCol]);
        else Transfer(fActive, fHidden, (*iter)[fIDCol]);
      }
    }
    ImGui::NextColumn();

    for(size_t col = 2; col < fCols->size(); ++col)
    {
      if(ImGui::Selectable(((*iter)[col]+"##"+idBase).c_str(), selected)) SelectID(id);
      ImGui::NextColumn();
    }
    
    //Scroll to this Node if it's selected
    //if(selected) ImGui::SetScrollHere();

    ImGui::Separator(); 

    return open;
  }

  //Draw a Node that I don't need to check whether its' children are selected
  void Scene::DrawNode(const mygl::TreeModel::iterator iter, const bool top, const size_t depth)
  {
    const bool open = DrawNodeData(iter, top);

    //Draw children of this Node
    if(open)
    {
      /*if(iter->NChildren() > 200)
      {
        //TODO: ImGuiListClipper does not work when a TreeNode() has siblings.  I need to re-engineer it somehow 
        //      to only take up the space I give it.  
        ImGuiListClipper clipper(iter->NChildren(), ImGui::GetTextLineHeightWithSpacing());
        while(clipper.Step())
        {
          const size_t start = clipper.DisplayStart;
          const size_t end = clipper.DisplayEnd;
          size_t pos = 0;
          auto child = iter->begin();
          for(; child != iter->end() && pos < end; ++child)
          {
            if(pos >= start)
            { 
              DrawNode(child, false);
            }
            ++pos;
          }

          //ListClipper doesn't handle drawing the last node of a list very well, so draw it myself
          //DrawNode(child, false);
        }
      }
      else*/
      {
        //Make sure column is wide enough for checkboxes
        if(open && ImGui::GetColumnWidth() < ImGui::GetTreeNodeToLabelSpacing()*(depth+1))
        {
          ImGui::SetColumnWidth(-1, ImGui::GetTreeNodeToLabelSpacing()*(depth+1)); //Leave extra space for the checkbox
        }

        for(auto child = iter->begin(); child != iter->end(); ++child)
        { 
          DrawNode(child, false, depth+1);
        }
      }

      ImGui::TreePop();
    }
  }
}
