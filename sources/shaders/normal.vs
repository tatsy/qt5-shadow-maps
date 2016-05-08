#version 330

in vec3 vertices;
in vec3 normals;
out vec3 normalWorldspace;

uniform mat4 mvpMat;

void main(void) {
    gl_Position = mvpMat * vec4(vertices, 1.0);
    normalWorldspace = normals;
}
