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
                                                      fSelection(), fModel(cols), fSelfCol(cols->fDrawSelf), fIDCol(cols->fVisID), 
                                                      fCols(cols), fCutBar(cols->size()), fBuffer(fCutBar.fInput), fVAO(new VAO()), 
                                                      fConfig(std::move(config))
  {
  }

  Scene::Scene(const std::string& name, const std::string& fragSrc, const std::string& vertSrc, const std::string& geomSrc, 
               std::shared_ptr<mygl::ColRecord>& cols, std::unique_ptr<SceneConfig>&& config): fName(name), fActive(), fHidden(), 
               fShader(fragSrc, vertSrc, geomSrc), fSelectionShader(INSTALL_GLSL_DIR "/selection.frag", vertSrc, geomSrc), fModel(cols), 
               fSelfCol(cols->fDrawSelf), fIDCol(cols->fVisID), fCols(cols), fCutBar(cols->size()), fBuffer(fCutBar.fInput), 
               fConfig(std::move(config))
  {
  }

  Scene::~Scene() {}

  TreeModel::iterator Scene::NewTopLevelNode()
  {
    return fModel.NewNode();
  }

  //Call this before Render() to get updates from user interaction with list tree.  
  void Scene::RenderGUI()
  {
    //Cut bar
    ImGui::InputText("##Cut", fBuffer.data(), fBuffer.size());
    ImGui::SameLine();
    if(ImGui::Button("Filter")) 
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
        fModel.Walk([this](const auto iter)
                    {
                      if(!fCutBar.do_filter(iter))
                      {
                        auto& visible = (*iter)[fSelfCol];
                        if(visible)
                        {
                          visible = false;
                          this->Transfer(fActive, fHidden, (*iter)[fIDCol]);
                        }
                      }
                    });
      }
      catch(const util::GenException& e)
      {
        std::cerr << "Caught exception during formula processing:\n" << e.what() << "\nIgnoring cuts for this Scene.\n";
      }
    }

    //Tree column labels
    ImGui::Columns(fCols->size()-1);
    ImGui::Text(fCols->Name(1).c_str());
    ImGui::NextColumn();
    for(size_t col = 2; col < fCols->size(); ++col) 
    {
      ImGui::Text(fCols->Name(col).c_str());
      //ImGui::SetColumnWidth(-1, ); //TODO: Make sure columns shown whole label and scroll
      ImGui::NextColumn();
    }
    ImGui::Separator();

    try
    {
      for(auto iter = fModel.begin(); iter != fModel.end(); ++iter) DrawNode(iter, true);
    }
    catch(const util::GenException& e)
    {
      std::cerr << "Caught std::exception " << e.what() << " when parsing formula.  Ignoring filter expression.\n";
    }
    ImGui::Columns(1); //Leave the multi-column context?
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
    fSelection = VisID();
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

  //TODO: switch to this Scene and tell other Scenes that this VisID has been selected
  //TODO: What happens when an object that is not visible in the 3D view is selected with the list tree? 
  //      Seems to usually be OK for now.  
  bool Scene::SelectID(const mygl::VisID& id)
  {
    //Find all objects with VisID id in the TreeModel.  Needed for generating a tooltip.
    auto result = fModel.end();
    fModel.Walk([&result, &id, this](const TreeModel::iterator& pos) 
                {
                  const bool found = ((mygl::VisID)((*pos)[this->fIDCol]) == id);
                  if(found) result = pos;
                  return found;
                });

    if(result != fModel.end())
    {
      //Highlight graphics object that was selected
      const auto id = (mygl::VisID)((*result)[this->fIDCol]);
      auto found = fActive.find(id);
      if(found != fActive.end())
      {
        found->second->SetBorder(0.01, glm::vec4(1., 0., 0., 1.));

        //Highlight selected objects in the TreeView
        //TODO: Restore this functionality with ImGUI
        //fTreeView.expand_to_path(result);
        //fTreeView.set_cursor(result);
  
        //TODO: overhaul signalling between Scenes and Viewers so that this only happens in one place.
        //Regardless of what was selected, unselect the last thing that was selected
        auto prev = fActive.find(fSelection); //The previously selected object might have been disabled since 
                                                    //it was selected.
        //TODO: Make border color and width a user parameter
        if(prev != fActive.end()) prev->second->SetBorder(0., glm::vec4(1., 0., 0., 1.));
        else
        {
          prev = fHidden.find(fSelection);
          if(prev != fHidden.end()) prev->second->SetBorder(0., glm::vec4(1., 0., 0., 1.));
        }

        //Mark this object as the new selection
        fSelection = id;
        return true;
      }
    }

    //return std::string("");
    return false;
  }

  void Scene::on_tree_selection()
  {
    //TODO: Restore this functionality with ImGUI
    /*const auto found = fTreeView.get_selection()->get_selected();

    //Regardless of what was selected, unselect the last thing that was selected
    auto prev = fActive.find(fSelection); //The previously selected object might have been disabled since 
                                                //it was selected.
    //TODO: Make border color and width a user parameter
    if(prev != fActive.end()) prev->second->SetBorder(0., glm::vec4(1., 0., 0., 1.));
    else
    {
      prev = fHidden.find(fSelection);
      if(prev != fHidden.end()) prev->second->SetBorder(0., glm::vec4(1., 0., 0., 1.));
    }

    if(!found) return;
    const auto id = (*found)[fIDCol];
    auto toHighlight = fActive.find(id);
    if(toHighlight != fActive.end()) toHighlight->second->SetBorder(0.01, glm::vec4(1., 0., 0., 1.));
    fSelection = id;*/
  }

  //TODO: The "top" bool is a hack to prevent the user from trying to draw top-level Nodes' objects. 
  //      So far, I have been using top-level nodes as placeholders.  
  void Scene::DrawNode(const mygl::TreeModel::iterator iter, const bool top)   
  {
    //if(!top && !fCutBar.do_filter(iter)) return;

    //This tree entry needs a unique ID for imgui.  Turn the 
    //VisID it includes into a string since I am not currently
    //putting the same VisID in more than one place in a given tree.
    std::stringstream ss;
    const auto& id = (*iter)[fIDCol];
    ss << id;
    const auto idBase = ss.str();

    //If this Node's VisID is selected, highlight this line in the list tree
    const bool selected = (id == fSelection); //TODO: Open also if a child is selected
    bool open = false;
    ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;    
    open = ImGui::TreeNodeEx(("##Node"+idBase).c_str(), node_flags);

    //If this Node was selected by a single click, select its' VisID.  
    if(ImGui::IsItemClicked()) SelectID(id);

    //Draw this Node's data
    //Draw a checkbox for the first column
    ImGui::SameLine(); //Put the checkbox on the same line as the tree arrow
    if(!top)
    {
      //std::cout << "Drawing checkbox.\n";
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

    //Draw children of this Node
    if(open) 
    {
      //TODO: Right now, frame rate really drops on restrictive cuts because I have to filter so many entries before 
      //      I get the ones clipper needs.  A couple of options might help here:
      //      1.) The equivalent of Gtk::TreeModelFilter.  Create a dummy TreeModel with only Nodes that pass cuts.  
      //          This might also give me the chance to address children that pass cuts when their parents don't.  
      //          Basically, caches the result of all of those do_filter() calls.  I really only need to store iterators to 
      //          the main TreeModel if I am careful...
      //      2.) Optimize for small numbers for NChildren.  I could probably guess a reasonable threshold for this.  
      //          I haven't yet thought of a strategy that gives me the number of children that pass cuts in time for 
      //          clipper's construction, but that might also help if I could make it work.  

      //TODO: Figure this number out programatically.  Measured it in an example for now
      if(iter->NChildren() > 100) //21 entries with max height on my laptop, but drawing 100 entries should not be a problem
      {  
        //First, let clipper figure out how many lines it wants
        ImGuiListClipper clipper(iter->NChildren());
        clipper.Step(); //clipper.StepNo == 0
        auto firstChild = iter->end();
        for(auto child = iter->begin(); child != iter->end(); ++child) 
        {
          if(fCutBar.do_filter(child)) 
          {
            DrawNode(child, false); //ImGuiListClipper really just needs me to draw one example line for step 0
            firstChild = child;
            break;
          }
        }
        clipper.Step(); //clipper.StepNo == 1
  
        //I now know how many lines clipper wants to draw.  Find enough children that pass cut bar to fill those lines
        size_t pos = 0;
        for(auto child = ++firstChild; child != iter->end() && pos != clipper.DisplayEnd; ++child)
        {
          if(fCutBar.do_filter(child)) //TODO: do_filter() is too slow for an imgui loop
          {
            ++pos;
            if(pos >= clipper.DisplayStart) DrawNode(child, false);
          }
        }
        clipper.Step(); //clipper.StepNo == 3
        ImGui::TreePop();
      }
      else //This is a short list, so no need for ListClipper
      {
        for(auto child = iter->begin(); child != iter->end(); ++child)
        {
          if(fCutBar.do_filter(child)) DrawNode(child, false);
        }
        ImGui::TreePop();
      }
    }
  }
}
