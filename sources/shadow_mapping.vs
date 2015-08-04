#version 330

in vec3 vertices;

uniform mat4 modelviewProjectionMatrix;

void main(void) {
  gl_Position = modelviewProjectionMatrix * vec4(vertices, 1.0);
}
