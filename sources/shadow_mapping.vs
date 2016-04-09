#version 330

in vec3 vertices;
in vec3 normals;
in vec3 colors;

uniform mat4 projectionMatrix;
uniform mat4 modelviewMatrix;
uniform mat4 normalMatrix;

out vec3 vertexWorldspace;
out vec3 normalWorldspace;
out vec3 vertexColor;

void main(void) {
  gl_Position = projectionMatrix * modelviewMatrix * vec4(vertices, 1.0);

  vertexWorldspace = vertices;
  normalWorldspace = normals;
  vertexColor = colors;
}
