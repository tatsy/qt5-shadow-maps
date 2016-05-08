#version 330

in vec3 vertices;
out float depth;

uniform mat4 mvpMat;

void main(void) {
    gl_Position = mvpMat * vec4(vertices, 1.0);
    depth = gl_Position.z / gl_Position.w;
}
