//File: Scene.cpp
//Brief: A Scene collects the resources to draw a group of related objects using opengl.   
//Author: Andrew Olivier aolivier@ur.rochester.edu

//view includes
#include "gl/Scene.h"
#include "gl/model/Drawable.h"

//c++ includes
#include <sstream>

namespace mygl
{
  //Warning: Make sure that Gtk::GLArea::make_current() is called just before this function so that 
  //         the proper GL context is bound for shader allocation.
  Scene::Scene(const std::string& name, const std::string& fragSrc, const std::string& vertSrc, std::shared_ptr<mygl::ColRecord>& cols): 
                                                                                                                        fName(name), fActive(), fHidden(), 
                                                                                                                        fShader(fragSrc, vertSrc), 
                                                                                                                        fSelectionShader("/home/aolivier/app/evd/src/gl/shaders/selection.frag", vertSrc),
                                                                                                                        fSelection(), fModel(cols), 
                                                                                                                        fSelfCol(cols->fDrawSelf),  
                                                                                                                        fIDCol(cols->fVisID) 
  {
    BuildGUI(cols);
  }

  Scene::Scene(const std::string& name, const std::string& fragSrc, const std::string& vertSrc, const std::string& geomSrc, 
               std::shared_ptr<mygl::ColRecord>& cols): fName(name), fActive(), fHidden(), fShader(fragSrc, vertSrc, geomSrc), 
               fSelectionShader("/home/aolivier/app/evd/src/gl/shaders/selection.frag", vertSrc, geomSrc), fModel(cols), 
               fSelfCol(cols->fDrawSelf), fIDCol(cols->fVisID)
  {
    BuildGUI(cols);
  }

  void Scene::BuildGUI(std::shared_ptr<mygl::ColRecord>& cols)
  {
    //fTreeView.set_model(fFilter);

    //TODO: Simplify this when I'm more comfortable with what's going on
    /*const int nTypes = cols.size();
    std::vector<std::string> types;
    for(int pos = 0; pos < nTypes; ++pos)
    {
      auto typeChars = g_type_name((cols.types())[pos]);
      std::string typeName(typeChars);
      //std::cout << g_type_name((cols.types())[pos]) << "\n";
      //std::cout << typeName << "\n";
      types.push_back(typeName);
    }
    fCutBar.SetTypes(types);*/

    //Standard columns.  fVisID is also present, but it should not be visible.
    //fTreeView.append_column_editable("Draw", fSelfCol);

    //auto renderer = ((Gtk::CellRendererToggle*)fTreeView.get_column_cell_renderer(0));
    //renderer->signal_toggled().connect(sigc::mem_fun(*this, &Scene::draw_self));

    //Configure filter
    //TODO: Replace this functionality with mygl::TreeModel::Walk()?
    //fFilter->set_visible_func(sigc::mem_fun(*this, &mygl::Scene::filter));
    //fCutBar.signal_activate().connect(sigc::mem_fun(*this, &Scene::start_filtering));

    //Make sure that objects that are selected in this TreeView are highlighted in the GUI
    //TODO: Notify other Scenes about what VisID was selected
    //fTreeView.get_selection()->signal_changed().connect(sigc::mem_fun(*this, &Scene::on_tree_selection));
  }

  Scene::~Scene() {}

  //Warning: Make sure that Gtk::GLArea::make_current() is called just before this function so that 
  //         the proper GL context is bound for resource allocation.
  TreeModel::iterator Scene::AddDrawable(std::unique_ptr<Drawable>&& drawable, const VisID& id, const TreeModel::iterator parent, 
                                         const bool active)
  {
    //Create a row in the TreeView for the drawable being added 
    auto iter = fModel.NewNode(parent);
    auto& row = *iter;

    //Make sure that a drawable with this VisID doesn't already exist
    auto exists = fActive.find(id);
    if(exists != fActive.end()) 
    {
      std::stringstream str;
      for(auto& pair: fActive) str << pair.first << "\n";
      throw util::GenException("Duplicate ID") << "In view::Scene::AddDrawable(), tried to add a new "
                                               << "drawable with ID " << id << " in scene " << fName 
                                               << ", but an object already exists with this ID!\n"
                                               << "Active IDs so far are:\n" << str.str() << "\n";
    }
    exists = fHidden.find(id);
    if(exists != fHidden.end()) 
    {
      std::stringstream str;
      for(auto& pair: fHidden) str << pair.first << "\n";
      throw util::GenException("Duplicate ID") << "In view::Scene::AddDrawable(), tried to add a new "
                                               << "drawable with ID " << id << " in scene " << fName
                                               << ", but an object already exists with this ID!\n"
                                               << "Hidden IDs so far are:\n" << str.str() << "\n";
    }
    else
    {
      if(active) fActive.emplace(id, std::move(drawable));
      else fHidden.emplace(id, std::move(drawable));
      row[fSelfCol] = active;
      row[fIDCol] = id;
    }
    return iter;
  }

  TreeModel::iterator Scene::NewTopLevelNode()
  {
    return fModel.NewNode();
  }

  //Warning: Make sure that Gtk::GLArea::make_current() is called just before this function so that 
  //         the proper GL context is bound for rendering.
  void Scene::Render(const glm::mat4& view, const glm::mat4& persp) 
  {
    //TODO: Draw TreeModel here

    //Note that these uniform names assume that like-named uniforms are 
    //handled by the shader programs used to form fShader
    fShader.Use();
    fShader.SetUniform("view", view);
    fShader.SetUniform("projection", persp);
    fShader.SetUniform("model", glm::mat4());  //In case Drawables don't set their own model matrices.  Setting the 
                                               //same uniform twice shouldn't be a problem, right?
    for(auto& pair: fActive) pair.second->Draw(fShader); //This is different from my old DRAWER contract because 
                                                         //Drawables are now responsible for binding their own 
                                                         //model matrices.
  }

  void Scene::RenderSelection(const glm::mat4& view, const glm::mat4& persp)
  {
    //Note that these uniform names assume that like-named uniforms are 
    //handled by the shader programs used to form fShader
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
  }

  //This function destroys Drawables that might (and probably should) make opengl calls in their destructors.  
  //Make sure that the openGL context for this Scene is current when this is called.  
  void Scene::RemoveAll() //Might be useful when updating event
  {
    //fCutBar.set_text(""); //TODO: Fix bug in CutBar so that it doesn't crash with cuts on 
                          //      numeric values propagated between calls to RemoveAll().
    fActive.erase(fActive.begin(), fActive.end());
    fHidden.erase(fHidden.begin(), fHidden.end());
    fModel.Clear();
    fSelection = VisID();
    //fModel->erase(fModel->get_iter("0")); //This was recommended online, but it doesn't change anything.
  }

  /*void Scene::draw_self(const Glib::ustring& path)
  {
    auto row = *(fModel.get_iter(path));
    if(!row) return;
    const auto id = row[fIDCol];
    const bool status = !row[fSelfCol]; //At the point that signal_toggled() is emitted, this entry has already been toggled to the 
                                        //opposite of what it was when the user selected it.

    //Do not attempt to show/hide top nodes
    if(row.parent())
    {
      if(status) Transfer(fActive, fHidden, id);
      else Transfer(fHidden, fActive, id);
    }
    //TODO: activate/deactivate children
  }*/

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

  //Intercept the result of UserCut::do_filter() to disable the Drawables for filtered rows.
  bool Scene::filter(const TreeModel::iterator& iter)
  {
    //std::cout << "Entering mygl::Scene::filter()\n";
    /*if(!iter) 
    {
      std::cerr << "Avoided processing a \"zombie\" row in mygl::Scene::filter().\n";
      return true; //TODO: I suspect that I am getting "Zombie" iterators in this function when I get many particles with 0 energy.
    }*/
    auto& row = *iter;
    //std::cout << "VisID was " << row[fIDCol] << "\n";
    //const bool result = fCutBar.do_filter(iter); //TODO: Even commenting this doesn't seem to solve my problem with the GUI not redrawing.
    const bool result = true;
    //std::cout << "Result from UserCut::do_filter() was " << std::boolalpha << (result?"true":"false") << "\n";
    if(result == false) 
    {
      //std::cout << "Cutting this row in mygl::Scene::filter().\n";
      //row[fSelfCol] = false; //TODO: This line causes a segmentation fault
                               //TODO: Adding seemingly unrelated code causes segmentation faults with otherwise-working configurations 
                               //      of the source code.  This sounds like a misuse of memory somewhere.  
                               //TODO: With new code in EvdWindow, I got an error about invalid iterators before a segmentation fault.  
                               //      This sounds like fFilter is getting corrupted somewhere
      //row.set_value(1, false); //This also causes a segmentation fault
      try
      {
        Transfer(fActive, fHidden, row[fIDCol]);
        row[fSelfCol] = false;
      }
      catch(const util::GenException& e)
      {
        //TODO: I am getting this exception often, even for things that shouldn't be filtered!  
        std::cerr << "Caught exception \n" << e.what() << "\n when trying to filter rows of TreeModel.  "
                  << "Ignoring this request to filter object.\n";
      }
    }
    //std::cout << "VisID is now " << row[fIDCol] << "\n";
    return result;
  }

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
}
