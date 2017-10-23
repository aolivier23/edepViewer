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
  Scene::Scene(const std::string& name, const std::string& fragSrc, const std::string& vertSrc, mygl::ColRecord& cols): fName(name), fActive(), fHidden(), 
                                                                                                                        fShader(fragSrc, vertSrc), 
                                                                                                                        fSelfCol(cols.fDrawSelf), 
                                                                                                                        fIDCol(cols.fVisID)
  {
    fModel = Gtk::TreeStore::create(cols);
    fTreeView.set_model(fModel);

    //Standard columns.  fVisID is also present, but it should not be visible.
    //TODO: TreeModelFilter is awesome!  Consider using it here.
    fTreeView.append_column_editable("Draw", fSelfCol);
    auto renderer = ((Gtk::CellRendererToggle*)fTreeView.get_column_cell_renderer(0));
    renderer->signal_toggled().connect(sigc::mem_fun(*this, &Scene::draw_self));
  }

  Scene::~Scene() {}

  //Warning: Make sure that Gtk::GLArea::make_current() is called just before this function so that 
  //         the proper GL context is bound for resource allocation.
  Gtk::TreeModel::Row Scene::AddDrawable(std::unique_ptr<Drawable>&& drawable, const VisID& id, const Gtk::TreeModel::Row& parent, 
                                         const bool active)
  {
    //Create a row in the TreeView for the drawable being added 
    auto row = *(fModel->append(parent->children()));

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
    return row;
  }

  Gtk::TreeModel::iterator Scene::NewTopLevelNode()
  {
    return fModel->append();
  }

  //Warning: Make sure that Gtk::GLArea::make_current() is called just before this function so that 
  //         the proper GL context is bound for rendering.
  void Scene::Render(const glm::mat4& view, const glm::mat4& persp) 
  {
    //Note that these uniform names assume that like-named uniforms are 
    //handled by the shader programs used to form fShader
    fShader.SetUniform("view", view);
    fShader.SetUniform("projection", persp);
    fShader.SetUniform("model", glm::mat4());  //In case Drawables don't set their own model matrices.  Setting the 
                                               //same uniform twice shouldn't be a problem, right?
    for(auto& pair: fActive) pair.second->Draw(fShader); //This is different from my old DRAWER contract because 
                                                         //Drawables are now responsible for binding their own 
                                                         //model matrices.
  }

  void Scene::RemoveAll() //Might be useful when updating event
  {
    fActive.erase(fActive.begin(), fActive.end());
    fModel->clear();
  }

  void Scene::draw_self(const Glib::ustring& path)
  {
    auto row = *(fModel->get_iter(path));
    const auto& id = row[fIDCol];
    const bool status = !row[fSelfCol]; //At the point that signal_toggled() is emitted, this entry has already been toggled to the 
                                        //opposite of what it was when the user selected it.
    if(status) Transfer(fActive, fHidden, id);
    else Transfer(fHidden, fActive, id);
    //TODO: activate/deactivate children
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
}
