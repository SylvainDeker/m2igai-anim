#version 140
#extension GL_ARB_explicit_attrib_location : require

layout (location = 0) in vec3 a_vertex;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in float a_weight0;
layout (location = 3) in float a_weight1;


uniform mat4 transform;
uniform mat4 child_bone;
uniform mat4 parent_bone;

out vec3 g_vertex;
out vec3 g_normal;
void main()
{
    vec4 tmp = transform*vec4(a_vertex,1.0);
    tmp = parent_bone * a_weight0 * tmp + parent_bone * child_bone * a_weight1 * tmp;
    tmp = tmp/tmp.w;

    gl_Position = tmp;
    g_vertex = tmp.xyz;
    g_normal = a_normal;
}
