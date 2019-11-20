#ifndef MINIMESH_H_
#define MINIMESH_H_

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include <glbinding/gl/gl.h>
#include <glbinding/ContextInfo.h>
#include <glbinding/Version.h>


#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <globjects/globjects.h>
#include <globjects/base/File.h>
#include <globjects/logging.h>
#include <globjects/Uniform.h>
#include <globjects/Program.h>
#include <globjects/Shader.h>

#include <globjects/VertexArray.h>
#include <globjects/VertexAttributeBinding.h>
#include <globjects/Buffer.h>

class MiniMesh : public globjects::Instantiator<MiniMesh>
{
public:

    struct Vertex {
        glm::vec3 pos;
        glm::vec3 normal;
        float weights[2]; // One weight to each handler
    };
    using Face = std::array<gl::GLuint, 3>;

public:
    MiniMesh(const std::vector<Vertex> &v, const std::vector<Face> &f);

    virtual ~MiniMesh(){};

    /** draws the mesh as single triangles
    */
    void draw();
    void draw(gl::GLenum mode);
    std::vector<Vertex> &vertices(){return m_vertices;}
    std::vector<Vertex> &restVertices(){return m_restVertices;}
    void updateVertices(const std::vector<Vertex> & vertex);

protected:
    std::vector<Vertex> m_vertices;
    std::vector<Vertex> m_restVertices;
    std::vector<Face> m_indices;
    gl::GLsizei m_size;
    std::unique_ptr<globjects::VertexArray> m_vao;
    std::unique_ptr<globjects::Buffer> m_gpuVertices;
    std::unique_ptr<globjects::Buffer> m_gpuIndices;
};



#endif
