//File: SceneController.cpp
//Brief: A SceneController collects the resources to draw a group of related objects using opengl.   
//Author: Andrew Olivier aolivier@ur.rochester.edu

//gl includes
#include "SceneController.h"
#include "gl/metadata/Column.cpp"
#include "SceneModel.cpp"
#include "gl/model/Drawable.h"
#include "SceneConfig.cpp"

//c++ includes
#include <sstream>
#include <array>
#include <algorithm>

//TODO: Remove me
#include <iostream>

namespace ctrl
{
  //TODO: Maybe just pass in a ShaderProg to simplify constructors?  Does it matter anymore when a ShaderProg is created?  
  SceneController::SceneController(const std::string& fragSrc, const std::string& vertSrc, 
                                   std::shared_ptr<ColumnModel>& cols, 
                                   std::unique_ptr<mygl::SceneConfig>&& config): 
                                                      fCutBar(cols->size()), fCols(cols), fSelectedColumn(std::numeric_limits<size_t>::max()),
                                                      fHistWindow(), fConfig(std::move(config)), fShader(fragSrc, vertSrc),
                                                      fSelectionShader(INSTALL_GLSL_DIR "/selection.frag", vertSrc)
  {
  }

  SceneController::SceneController(const std::string& fragSrc, const std::string& vertSrc, const std::string& geomSrc, 
               std::shared_ptr<ColumnModel>& cols, std::unique_ptr<mygl::SceneConfig>&& config): fCutBar(cols->size()), 
               fCols(cols), fSelectedColumn(std::numeric_limits<size_t>::max()), fHistWindow(), fConfig(std::move(config)),
               fShader(fragSrc, vertSrc, geomSrc), fSelectionShader(INSTALL_GLSL_DIR "/selection.frag", vertSrc, geomSrc)
  {
  }

  SceneController::~SceneController() {}

  void SceneController::NewEvent(std::unique_ptr<model_t>&& newModel, mygl::VisID& nextID)
  {
    fCurrentModel = std::move(newModel);
    
    //Set VisIDs for the entire model in a predictable pattern. 
    for(auto& top: fCurrentModel->fTopLevelNodes) top.walk([this, &nextID](auto& child) { child.fVisID = nextID++; });
    fVAO.Load(fCurrentModel->fVAO);

    //"remember" cut settings from last event
    fCutBar.ApplyCut(fCurrentModel->fTopLevelNodes);

    //Cache the last VisID in this scene for this event
    fLastID = nextID;
  }

  //Call this before Render() to get updates from user interaction with list tree.  
  void SceneController::RenderGUI()
  {
    fCutBar.Render(fCurrentModel->fTopLevelNodes);

    //Tree column labels
    //Calculate the total length of text I will want to display
    float width = 5.*ImGui::GetTreeNodeToLabelSpacing(); //Make space for 5 tree node opens
    for(size_t col = 0; col < fCols->size(); ++col)
    {
      width += 2.*ImGui::CalcTextSize(fCols->Name(col).c_str()).x + ImGui::GetStyle().ItemSpacing.x;
    }

    ImGui::SetNextWindowContentSize(ImVec2(width, 0)); //Make space for 5 tree node opens plus column labels
    ImGui::BeginChild("List Tree", ImVec2(0,0), true, ImGuiWindowFlags_HorizontalScrollbar);
    ImGui::Columns(fCols->size()+1);
    ImGui::Text("Visible?");
    ImGui::NextColumn();

    bool selected = false;
    for(size_t col = 0; col < fCols->size(); ++col) 
    {
      selected = (col == fSelectedColumn);
      ImGui::Selectable(fCols->Name(col).c_str(), &selected);
      if(selected)
      {
        fSelectedColumn = col;
        if(!fHistWindow.Render(fCurrentModel->fTopLevelNodes, col, fCols->Name(col))) fSelectedColumn = std::numeric_limits<size_t>::max();
      }
      ImGui::NextColumn();
    }
    ImGui::Separator();

    try
    {
      for(auto& top: fCurrentModel->fTopLevelNodes) 
      {
        top.walkIf([this](auto& node)
                   {
                     if(this->DrawNodeData(node))
                     {
                       //Make sure column is wide enough for checkboxes
                       if(ImGui::GetColumnWidth() < ImGui::GetTreeNodeToLabelSpacing())
                       {
                         ImGui::SetColumnWidth(-1, ImGui::GetTreeNodeToLabelSpacing());
                       }

                       return true;
                     }
                     return false;
                   }, [](auto& /*node*/) { ImGui::TreePop(); });
      }
    }
    catch(const util::GenException& e)
    {
      std::cerr << "Caught std::exception " << e.what() << " when parsing formula.  Ignoring filter expression.\n";
    }
    ImGui::Columns(1); //Leave the multi-column context?
    ImGui::EndChild();
  }

  void SceneController::Render(const glm::mat4& view, const glm::mat4& persp) 
  {
    //TODO: Why isn't anything visible?  Even the grid doesn't show up, but something 
    //      seems to be happening because the frame rate drops dramatically when I turn 
    //      on the 1mm grid.
    //Note that these uniform names assume that like-named uniforms are 
    //handled by the shader programs used to form fShader
    fConfig->BeforeRender();
    auto bound = fVAO.Use();
    fShader.Use();
    fShader.SetUniform("view", view);
    fShader.SetUniform("projection", persp);
    fShader.SetUniform("model", glm::mat4());  //In case Drawables don't set their own model matrices.  Setting the 
                                               //same uniform twice shouldn't be a problem, right?
    for(auto& top: fCurrentModel->fTopLevelNodes) 
    {
      //top-level nodes have special meaning.  Don't try to Draw() their handles.  
      if(top.fVisible)
      {
        for(auto& child: top.children)
        {
          child.walk([this](auto& node)
                     {
                       if(node.fVisible) node.handle->Draw(fShader);
                     });
        }
      }
    }
    fConfig->AfterRender();
  }

  void SceneController::RenderSelection(const glm::mat4& view, const glm::mat4& persp)
  {
    //Note that these uniform names assume that like-named uniforms are 
    //handled by the shader programs used to form fShader
    auto bound = fVAO.Use();
    fSelectionShader.Use();
    fSelectionShader.SetUniform("view", view);
    fSelectionShader.SetUniform("projection", persp);
    fSelectionShader.SetUniform("model", glm::mat4());  //In case Drawables don't set their own model matrices.  Setting the 
                                                        //same uniform twice shouldn't be a problem, right?

    for(auto& top: fCurrentModel->fTopLevelNodes)
    {
      if(top.fVisible)
      {
        for(auto& child: top.children)
        {
          child.walk([this](auto& node) 
                     {
                       if(node.fVisible)
                       {
                         fSelectionShader.SetUniform("idColor", node.fVisID); //Each VisID is a unique color that can be drawn by opengl.  
                                                                              //So, draw this object with that color so that its' color 
                                                                              //can be mapped back to its' VisID if the user clicks on it.
                         node.handle->Draw(fSelectionShader);
                       }
                     });
        }
      }
    }
  }

  //TODO: Tell other SceneControllers that this VisID has been selected
  bool SceneController::SelectID(const mygl::VisID& searchID)
  {
    //TODO: Rewrite this function to take advantage of list model instead.  If an object isn't 
    //      visible, then no need to check whether its children are selected.  Furthermore, since 
    //      I set VisIDs, I can ensure that VisIDs between siblings are children of the "lhs".  
    //      I think this lets me write an algorithm something like a binary search tree (this one 
    //      isn't binary though).  For the visibility check, I'd need something like walkWhileTrueIf().  
    //Regardless of what was selected, unselect the last thing that was selected
    mygl::VisID oldSelected;
    if(!fSelectPath.empty()) 
    {
      oldSelected = fSelectPath.front(); 
      if(oldSelected == searchID) return true; //If the same object was selected twice in a row, we've found
                                               //and selected it with no effort!
    }
    fSelectPath.clear();
    auto& top = fCurrentModel->fTopLevelNodes;

    const auto compare = [](const auto& compareTo, const auto& node) { return compareTo < node.fVisID; };
    const auto parentOfOldSelected = std::upper_bound(top.begin(), top.end(), oldSelected, compare);
    if(parentOfOldSelected != top.begin())
    {
      //TODO: If selection is very slow, this is the first place to suspect.  A good 
      //      alternative would be caching a handle somehow.
      std::prev(parentOfOldSelected)->search([this, &oldSelected](auto& node)
                                             {
                                               if(node.handle && node.fVisID == oldSelected) node.handle->SetBorder(0., glm::vec4(1., 0., 0., 1.));
                                             }, oldSelected);
    }

    //If searchID can't possibly be in this scene, return now
    if(!(searchID < fLastID))
    {
      return false;
    }
                                             
    //Now, find the next object to be selected
    const auto parentOfSelected = std::upper_bound(top.begin(), top.end(), searchID, compare);
    if(parentOfSelected == top.begin()) 
    {
      return false;
    }
 
    //Since fVisID assignment descends the list tree before going to the next node, all children of a given node are ordered by 
    //fVisID.  So, if I ever reach a point where walkWhileTrue() returns false, I've either found searchID, or it isn't in 
    //fCurrentModel at all.  
    if(!std::prev(parentOfSelected)->search([this, &searchID](auto& node)
                                            {
                                              fSelectPath.push_back(node.fVisID);
                                              if(node.fVisID == searchID) node.handle->SetBorder(0.01, glm::vec4(1., 0., 0., 1.));
                                            }, searchID))
    {
      fSelectPath.clear();
      return false;
    }
    return true;
  }

  bool SceneController::DrawNodeData(node_t& node)
  {
    //This tree entry needs a unique ID for imgui.  Turn the 
    //VisID it includes into a string since I am not currently
    //putting the same VisID in more than one place in a given tree.
    std::stringstream ss;
    const auto& id = node.fVisID;
    ss << id;
    const auto idBase = ss.str();

    //If this Node's VisID is selected, highlight this line in the list tree
    const bool selected = (!fSelectPath.empty()) && (id == fSelectPath.front()); 
    bool open = false;
    ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;    
    if(node.children.size() == 0) node_flags |= ImGuiTreeNodeFlags_Leaf;
    if(!selected && std::binary_search(fSelectPath.rbegin(), fSelectPath.rend(), id)) node_flags |= ImGuiTreeNodeFlags_DefaultOpen;
    //if(std::find(fSelectPath.begin(), fSelectPath.end(), id) != fSelectPath.end()) node_flags |= ImGuiTreeNodeFlags_DefaultOpen;
    //TODO: I know the top-level node whose child is selected, so I only need to do this for a very small number of nodes
    open = ImGui::TreeNodeEx(("##Node"+idBase).c_str(), node_flags);

    //Draw this Node's data
    //Draw a checkbox for the first column
    ImGui::SameLine(); //Put the checkbox on the same line as the tree arrow
    if(ImGui::Checkbox(("##Check"+idBase).c_str(), &(node.fVisible))) //If this node's visibility was toggled, toggle all children to the same
    {
      const bool visible = node.fVisible;
      node.walk([&visible](auto& child) { child.fVisible = visible; });
    }
    ImGui::NextColumn();

    for(size_t col = 0; col < fCols->size(); ++col)
    {
      if(ImGui::Selectable((node.row[col]+"##"+idBase).c_str(), selected)) SelectID(id);
      ImGui::NextColumn();
    }
    
    //Scroll to this Node if it's selected
    //if(selected) ImGui::SetScrollHere();

    ImGui::Separator(); 

    return open;
  }
}
