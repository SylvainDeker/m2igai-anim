#include "MiniMesh.hpp"


MiniMesh::MiniMesh(const std::vector<Vertex> &v, const std::vector<Face> &f):
    m_vao(globjects::VertexArray::create())
    , m_gpuVertices(globjects::Buffer::create())
    , m_gpuIndices(globjects::Buffer::create())
{
    m_indices.insert(m_indices.begin(), f.begin(), f.end());
    m_restVertices.insert(m_vertices.begin(), v.begin(), v.end());
    m_vertices.insert(m_vertices.begin(), v.begin(), v.end());

    m_gpuIndices->setData(m_indices, gl::GL_STATIC_DRAW);
    m_gpuVertices->setData(m_vertices, gl::GL_STATIC_DRAW);

    m_size = static_cast<gl::GLsizei>(m_indices.size() * 3);

    m_vao->bindElementBuffer(m_gpuIndices.get());

    {
        auto vertexBinding = m_vao->binding(0);
        vertexBinding->setAttribute(0);
        vertexBinding->setBuffer(m_gpuVertices.get(), 0, sizeof(Vertex));
        vertexBinding->setFormat(3, gl::GL_FLOAT, gl::GL_FALSE);
        m_vao->enable(0);
    }
    {
        auto vertexBinding = m_vao->binding(1);
        vertexBinding->setAttribute(1);
        vertexBinding->setBuffer(m_gpuVertices.get(), offsetof(Vertex, normal), sizeof(Vertex));
        vertexBinding->setFormat(3, gl::GL_FLOAT, gl::GL_TRUE);
        m_vao->enable(1);
    }
    {
        auto vertexBinding = m_vao->binding(2);
        vertexBinding->setAttribute(2);
        vertexBinding->setBuffer(m_gpuVertices.get(), offsetof(Vertex, weights[0]), sizeof(Vertex));
        vertexBinding->setFormat(1, gl::GL_FLOAT, gl::GL_TRUE);
        m_vao->enable(2);
    }
    {
        auto vertexBinding = m_vao->binding(3);
        vertexBinding->setAttribute(3);
        vertexBinding->setBuffer(m_gpuVertices.get(), offsetof(Vertex, weights[1]), sizeof(Vertex));
        vertexBinding->setFormat(1, gl::GL_FLOAT, gl::GL_TRUE);
        m_vao->enable(3);
    }

    m_vao->unbind();
}


void MiniMesh::draw()
{
    draw(gl::GL_TRIANGLES);
}

void MiniMesh::draw(const gl::GLenum mode)
{
    glEnable(gl::GL_DEPTH_TEST);

    m_vao->bind();
    m_vao->drawElements(mode, m_size, gl::GL_UNSIGNED_INT, nullptr);
    m_vao->unbind();
}

void MiniMesh::updateVertices(const std::vector<Vertex>& vertex){
    m_gpuVertices->setData(vertex, gl::GL_STATIC_DRAW);
    auto vertexBinding = m_vao->binding(0);
    vertexBinding->setBuffer(m_gpuVertices.get(), 0, sizeof(Vertex));
}
