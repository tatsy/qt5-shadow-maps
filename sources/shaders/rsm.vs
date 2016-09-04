#version 330

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec4 in_color;

out vec3 f_posWorld;
out vec3 f_nrmWorld;
out vec4 f_posScreen;
out vec4 f_color;

uniform mat4 u_mvpMat;

void main(void) {
    gl_Position = u_mvpMat * vec4(in_position, 1.0);

    f_posWorld = in_position;
    f_nrmWorld = in_normal;
    f_posScreen = gl_Position;
    f_color = in_color;
}
