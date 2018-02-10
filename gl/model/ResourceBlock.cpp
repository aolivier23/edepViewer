//File: ResourceBlock.cpp
//Brief: A ResourceBlock accumulates the opengl resources for a group of Drawables and sends them to the GPU at the same time.  
//       Each Drawable's constructor should be passed a ResourceBlock, and that Drawable should Add() a vector<Vertex> and a 
//       vector of indices to the ResourceBlock in exchange for a Resource.  The ResourceBlock will bind its' vertex array to 
//       prepare for its' Drawables to be drawn.  It will deallocate its' VAO, VBO, and EBO in its' destructor.  
//Author: Andrew Olivier aolivier@ur.rochester.edu
//TODO: Path and Point don't use EBOs.  Probably make this a class template and specialize for them.
//TODO: Default shader program for each Drawable?  This might be a good place for that information if I make this a class template.   

namespace mygl
{
    //Allocate opengl resources
    ResourceBlock(std::vector<Drawable>&& drawables): fVertices()
    {
      //Get some opengl resources right away since I only need one of each.
      glGenVertexArrays(1, &fVAO);
      glBindVertexArray(fVAO);

      glGenBuffers(1, &fEBO); //TODO: Some Drawables, like Point and Path, don't use EBOs at all.  Do I need to resort to the template?
      glGenBuffers(1, &fVBO);

      //Specify how to break down a Vertex object into arrays for the shaders
      glEnableVertexAttribArray(0);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Drawable::Vertex), (GLvoid*)(0));

      glEnableVertexAttribArray(1);
      glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Drawable::Vertex), (GLvoid*)(sizeof(glm::vec3)));

      //Unbind my vertex array object to prevent potential mistakes
      glBindVertexArray(0);
    }

    //Accept Vertices from a Drawable and return the index of its' Vertices in this pool of data
    GLuint Add(std::vector<Drawable::Vertex>&& vertices, std::vector<GLuint>&& indices)
    {
      //TODO: Does it slow me down much if I just buffer these right away?  I need to test this once I have 
      //      ResourceBlock working.  I'd rather bind them right away so that the memory only has to exist 
      //      on the GPU.
      GLuint pos = fVertices.size();
      fVertices.insert(fVertices.end(), vertices.begin(), vertices.end());
      fIndices.insert(fIndices.end(), indices.begin(), indices.end());
      return pos;
    }

    //Finish adding Drawables and send data to the GPU.
    //TODO: I might be able to remove this altogether if I find out I can just send vertex data to the GPU immediately at 
    //      negligible cost.  Then, I would call these functions in Add() instead of after all Add()s are done. 
    void Finish()
    {
      //Tell opengl what vertices I am using
      glBindVertexArray(fVAO);

      //Send indices to the GPU
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, fEBO);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, flatIndices.size()*sizeof(GLuint), &flatIndices[0], GL_STATIC_DRAW);

      //Send vertices to the GPU
      glBindBuffer(GL_ARRAY_BUFFER, fVBO);
      glBufferData(GL_ARRAY_BUFFER, vertexData.size()*sizeof(Vertex), &vertexData[0], GL_STATIC_DRAW);
      
      //Unbind vertex array to prevent mistakes
      glBindVertexArray(0);
    }

    //Make this ResourceBlock's resources the current resources on the GPU
    void Bind()
    {
      //Tell opengl what vertices I am using
      glBindVertexArray(fVAO);
    }

    //Undo a Bind() to get ready for the next ResourceBlock
    void Unbind()
    {
      //Unbind vertex array to prevent mistakes
      glBindVertexArray(0);
    }

    //Deallocate opengl resources and destroy Drawables
    virtual ~ResourceBlock()
    {
      glDeleteVertexArrays(1, &fVAO);
      glDeleteBuffers(1, &fVBO);
      glDeleteBuffers(1, &fEBO);
    }

    private:
      GLuint fVAO; //opengl Vertex Array Object name
      GLuint fVBO; //opengl Vertex Buffer Object name
      GLuint fEBO; //opengl Element Buffer Object name

      //TODO: I'd rather hold on to these Vertices for as little time as possible.  Better to be 
      //      more memory-efficient.  I can experiment with binding Vertices as I get them once I have 
      //      ResourceBlock working.  
      std::vector<Drawable::Vertex> fVertices; //List of vertices to be bound to a VBO for drawing
      std::vector<GLuint> fIndices; //List of indices to be bound to an EBO for drawing
}
