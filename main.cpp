
#include <iostream>
#include <chrono>
#include <algorithm>

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

#include "MiniMesh.hpp"
#include "Bone.hpp"


Bone parent_bone;
Bone child_bone;

bool g_gpu;

std::unique_ptr<MiniMesh> makeCylinder(glm::vec3 base=glm::vec3(0,0,0), glm::vec3 axis = glm::vec3(1,0,0), float radius = .5f, float length = 3., int subdiv1 = 64, int subdiv2 = 512){

    std::vector<MiniMesh::Vertex> vertices;
    std::vector<MiniMesh::Face> indices;
    glm::vec3 x = glm::vec3(0,1,0); //orthogonal(axis);
    glm::vec3 y = cross(axis, x);

    for(int i=0; i<subdiv2; i++)
    {
        float offset = float(i)/float(subdiv2-1);
        float offset2 = (offset-0.5)*length;
        for(int j=0; j<subdiv1; j++)
        {
            float angle = 2.*glm::pi<float>()*float(j)/float(subdiv1);
            MiniMesh::Vertex nv;
            nv.pos = base+offset2*axis+radius*glm::cos(angle)*x+radius*glm::sin(angle)*y;
            nv.normal = glm::normalize(glm::cos(angle)*x+glm::sin(angle)*y);
            nv.weights[0] = float(i)/subdiv2; // Handler 1
            nv.weights[1] = 1-nv.weights[0]; // Handler 2
            vertices.push_back(nv);
        }
    }


    for(unsigned int i=0; i<subdiv2-1; i++)
    {
        for(unsigned int j=0; j<subdiv1; j++)
        {
            MiniMesh::Face f1 {{i*subdiv1+j,i*subdiv1+(j+1)%subdiv1,i*subdiv1+j+subdiv1}};
            indices.push_back(f1);
            MiniMesh::Face f2 {{i*subdiv1+(j+1)%subdiv1,i*subdiv1+j+subdiv1, i*subdiv1+(j+1)%subdiv1+subdiv1}};
            indices.push_back(f2);
        }

    }

    return MiniMesh::create(vertices, indices);
}


namespace
{
    std::unique_ptr<globjects::Program> g_program = nullptr;
    std::unique_ptr<globjects::File> g_vertexShaderSource = nullptr;
    std::unique_ptr<globjects::AbstractStringSource> g_vertexShaderTemplate = nullptr;
    std::unique_ptr<globjects::Shader> g_vertexShader = nullptr;
    std::unique_ptr<globjects::File> g_fragmentShaderSource = nullptr;
    std::unique_ptr<globjects::AbstractStringSource> g_fragmentShaderTemplate = nullptr;
    std::unique_ptr<globjects::Shader> g_fragmentShader = nullptr;
    std::unique_ptr<globjects::File> g_phongShaderSource = nullptr;
    std::unique_ptr<globjects::AbstractStringSource> g_phongShaderTemplate = nullptr;
    std::unique_ptr<globjects::Shader> g_phongShader = nullptr;

    std::unique_ptr<MiniMesh> g_mesh = nullptr;
    glm::mat4 g_viewProjection;

    const std::chrono::high_resolution_clock::time_point g_starttime = std::chrono::high_resolution_clock::now();

    auto g_size = glm::ivec2{};
}

void resize()
{
    static const auto fovy  = glm::radians(40.f);
    static const auto zNear  = 1.f;
    static const auto zFar   = 16.f;
    static const auto eye    = glm::vec3{ 0.f, 1.f, 8.f };
    static const auto center = glm::vec3{ 0.0, 0.0, 0.0 };
    static const auto up     = glm::vec3{ 0.0, 1.0, 0.0 };

    const auto aspect = static_cast<float>(g_size.x) / glm::max(static_cast<float>(g_size.y), 1.f);

    g_viewProjection = glm::perspective(fovy, aspect, zNear, zFar) * glm::lookAt(eye, center, up);
}

void initialize()
{
    const auto dataPath = std::string("./Shaders/");

    parent_bone.transform = glm::mat4(1.0);
    child_bone.transform = glm::mat4(1.0);


    g_program = globjects::Program::create();

    g_vertexShaderSource = globjects::Shader::sourceFromFile(dataPath + (g_gpu?"./basic_gpu.vert":"./basic.vert"));
    g_vertexShaderTemplate = globjects::Shader::applyGlobalReplacements(g_vertexShaderSource.get());
    g_vertexShader = globjects::Shader::create(gl::GL_VERTEX_SHADER, g_vertexShaderTemplate.get());

    g_fragmentShaderSource = globjects::Shader::sourceFromFile(dataPath + "./basic.frag");
    g_fragmentShaderTemplate = globjects::Shader::applyGlobalReplacements(g_fragmentShaderSource.get());
    g_fragmentShader = globjects::Shader::create(gl::GL_FRAGMENT_SHADER, g_fragmentShaderTemplate.get());

    g_phongShaderSource = globjects::Shader::sourceFromFile(dataPath + "./phong.frag");
    g_phongShaderTemplate = globjects::Shader::applyGlobalReplacements(g_phongShaderSource.get());
    g_phongShader = globjects::Shader::create(gl::GL_FRAGMENT_SHADER, g_phongShaderTemplate.get());

    g_program->attach(
        g_vertexShader.get(),
        g_fragmentShader.get(),
        g_phongShader.get()
    );

    g_mesh = makeCylinder();

    g_mesh->updateVertices(g_mesh->vertices());

    resize();
}

void deinitialize()
{
    g_program.reset(nullptr);

    g_program.reset(nullptr);
    g_vertexShaderSource.reset(nullptr);
    g_vertexShaderTemplate.reset(nullptr);
    g_vertexShader.reset(nullptr);
    g_fragmentShaderSource.reset(nullptr);
    g_fragmentShaderTemplate.reset(nullptr);
    g_fragmentShader.reset(nullptr);
    g_phongShaderSource.reset(nullptr);
    g_phongShaderTemplate.reset(nullptr);
    g_phongShader.reset(nullptr);

    g_mesh.reset(nullptr);
}

void draw()
{
    gl::glClearColor(0.01,0.1,0.1,1);
    gl::glClear(gl::GL_COLOR_BUFFER_BIT | gl::GL_DEPTH_BUFFER_BIT);

    const auto t_elapsed = std::chrono::high_resolution_clock::now() - g_starttime;
    const auto t = static_cast<float>(t_elapsed.count()) * 4e-10f;

    g_program->setUniform("transform", g_viewProjection);

    if(g_gpu){
      g_program->setUniform("child_bone", child_bone.transform);
      g_program->setUniform("parent_bone", parent_bone.transform);
    }

    const auto level = static_cast<int>((glm::sin(t) * 0.5f + 0.5f) * 16) + 1;
    g_program->setUniform("level", level);
    g_program->use();

    gl::glViewport(0, 0, g_size.x, g_size.y);

    g_mesh->draw(gl::GL_TRIANGLES);

    g_program->release();
}


void error(int errnum, const char * errmsg)
{
    globjects::critical() << errnum << ": " << errmsg << std::endl;
}

void framebuffer_size_callback(GLFWwindow * /*window*/, int width, int height)
{
    g_size = glm::ivec2{ width, height };
    resize();
}

void reload(float parent, float child) {

  parent_bone.transform = glm::rotate(parent_bone.transform,parent,glm::vec3(0,0,1));
  child_bone.transform = glm::rotate( child_bone.transform,child,glm::vec3(0,0,1));

  if( ! g_gpu){
    glm::vec4 tmp;
    for (unsigned int i = 0; i < g_mesh->restVertices().size(); i++) {
      tmp = glm::vec4(g_mesh->restVertices()[i].pos,1.0);
      tmp = parent_bone.transform * g_mesh->restVertices()[i].weights[0] * tmp
            + parent_bone.transform * child_bone.transform * g_mesh->restVertices()[i].weights[1] * tmp;
      g_mesh->vertices()[i].pos = glm::vec3(tmp.x,tmp.y,tmp.z)/tmp.w;
    }
  }

}

void key_callback(GLFWwindow * window, int key, int /*scancode*/, int action, int /*modes*/)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE)
        glfwSetWindowShouldClose(window, true);

    if (key == GLFW_KEY_F5 && action == GLFW_RELEASE)
    {
        g_vertexShaderSource->reload();
        g_fragmentShaderSource->reload();
        g_phongShaderSource->reload();
    }
    float scale = 0.1;
    if (key == GLFW_KEY_F1) reload(scale,0.0);
    if (key == GLFW_KEY_F2) reload(-scale,0.0);
    if (key == GLFW_KEY_F3) reload(0.0,-scale);
    if (key == GLFW_KEY_F4) reload(0.0,scale);

    g_mesh->updateVertices(g_mesh->vertices());
}




int main(int argc, char * argv[])
{
    if( argc!= 2){
      std::cerr << "Usage: "<< argv[0] << " -gpu | -cpu" << '\n';
      exit(-1);
    }

    if(argv[1][1]=='g'){
      g_gpu = true;
    }

    // Initialize GLFW
    if (!glfwInit())
        return 1;

    glfwSetErrorCallback(error);
    glfwDefaultWindowHints();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, true);

    // Create a context and, if valid, make it current
    GLFWwindow * window = glfwCreateWindow(640, 480, "globjects Tessellation", NULL, NULL);
    if (window == nullptr)
    {
        globjects::critical() << "Context creation failed. Terminate execution.";

        glfwTerminate();
        return -1;
    }
    glfwSetKeyCallback(window, key_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glfwMakeContextCurrent(window);

    // Initialize globjects (internally initializes glbinding, and registers the current context)
    globjects::init();

    std::cout << std::endl
        << "OpenGL Version:  " << glbinding::ContextInfo::version() << std::endl
        << "OpenGL Vendor:   " << glbinding::ContextInfo::vendor() << std::endl
        << "OpenGL Renderer: " << glbinding::ContextInfo::renderer() << std::endl
        << "Annimation GPU : " << (g_gpu?"ON":"OFF") << std::endl;

    globjects::info() << "Press F5 to reload shaders." << std::endl << std::endl;

    glfwGetFramebufferSize(window, &g_size[0], &g_size[1]);
    initialize();

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        draw();
        glfwSwapBuffers(window);
    }
    deinitialize();

    // Properly shutdown GLFW
    glfwTerminate();

    return 0;
}
