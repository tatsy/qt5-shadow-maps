#version 330

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec4 in_color;

out vec3 g_position;
out vec3 g_normal;
out vec4 g_color;

void main(void) {
    g_position = in_position;
    g_normal = in_normal;
    g_color = in_color;
}
