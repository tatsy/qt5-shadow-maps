#version 330

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec4 in_color;

out vec3 f_posView;
out vec3 f_nrmView;
out vec3 f_lightPos;
out vec3 f_color;

out vec3 f_posWorld;
out vec3 f_nrmWorld;
out vec4 f_posLightSpace;

uniform mat4 u_mvMat;
uniform mat4 u_mvpMat;
uniform vec3 u_lightPos;

uniform mat4 u_lightMvpMat;

void main(void) {
    gl_Position = u_mvpMat * vec4(in_position, 1.0);

    f_posView = (u_mvMat * vec4(in_position, 1.0)).xyz;
    f_nrmView = (transpose(inverse(u_mvMat)) * vec4(in_normal, 1.0)).xyz;
    f_lightPos = (u_mvMat * vec4(u_lightPos, 1.0)).xyz;
    f_color   = in_color.rgb;

    f_posWorld = in_position;
    f_nrmWorld = in_normal;
    f_posLightSpace = u_lightMvpMat * vec4(in_position, 1.0);
}
