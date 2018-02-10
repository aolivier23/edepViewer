//File: ResourceBlock.h
//Brief: A ResourceBlock accumulates the opengl resources for a group of Drawables and sends them to the GPU at the same time.  
//       Each Drawable's constructor should be passed a ResourceBlock, and that Drawable should Add() a vector<Vertex> and a 
//       vector of indices to the ResourceBlock in exchange for a Resource.  The ResourceBlock will bind its' vertex array to 
//       prepare for its' Drawables to be drawn.  It will deallocate its' VAO, VBO, and EBO in its' destructor.  
//Author: Andrew Olivier aolivier@ur.rochester.edu

namespace mygl
{
  //If different Drawables ended up wanting to use different Vertex types or get 
  //GL resources in different ways, this design could be adapted by writing a 
  //class template that can be specialized.  
  class ResourceBlock
  {
    public:
      //Allocate opengl resources
      ResourceBlock(std::vector<Drawable>&& drawables);

      //Accept Vertices from a Drawable and return the index of its' Vertices in this pool of data
      GLuint Add(std::vector<Drawable::Vertex>&& vertices);

      //Finish adding Drawables and send data to the GPU.
      void Finish();

      //Make this ResourceBlock's resources the current resources on the GPU
      void Bind();

      //Undo a Bind() to get ready for the next ResourceBlock
      void Unbind();

      //Deallocate opengl resources and destroy Drawables
      virtual ~ResourceBlock();

    private:
      GLuint fVAO; //opengl Vertex Array Object name
      GLuint fVBO; //opengl Vertex Buffer Object name
      GLuint fEBO; //opengl Element Buffer Object name

      //TODO: I'd rather hold on to these Vertices for as little time as possible.  Better to be 
      //      more memory-efficient.  I can experiment with binding Vertices as I get them once I have 
      //      ResourceBlock working.  
      std::vector<Drawable::Vertex> fVertices; //List of vertices to be bound to a VBO for drawing
  };
}
