#version 330

in vec3 vertices;

void main(void) {
  gl_Position = vec4(vertices, 1.0);
}
