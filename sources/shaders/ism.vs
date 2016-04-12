#version 330

in vec3 vertices;
in vec3 normals;
in vec3 colors;

void main(void) {
  gl_Position = vec4(vertices, 1.0);
}
