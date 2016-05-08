#version 330

in vec3 vertices;
in vec3 normals;
in vec3 colors;

uniform mat4 projectionMatrix;
uniform mat4 modelviewMatrix;
uniform mat4 normalMatrix;
uniform mat4 depthMVP;

uniform vec3 lightPosition;

out vec3 vertexWorldspace;
out vec3 normalWorldspace;
out vec3 lightWorldspace;

out vec3 vertexCameraspace;
out vec3 normalCameraspace;
out vec3 lightCameraspace;
out vec3 vertexColor;

out vec4 vertexScreenspace;

out vec4 shadowCoord;

void main(void) {
  gl_Position = projectionMatrix * modelviewMatrix * vec4(vertices, 1.0);

  vertexWorldspace = vertices;
  normalWorldspace = normals;
  lightWorldspace = lightPosition;

  vertexCameraspace = (modelviewMatrix * vec4(vertices, 1.0)).xyz;
  normalCameraspace = (normalMatrix * vec4(normals, 1.0)).xyz;
  lightCameraspace = (modelviewMatrix * vec4(lightPosition, 0.0)).xyz;

  vertexColor = colors;

  vertexScreenspace = gl_Position;

  shadowCoord = depthMVP * vec4(vertices, 1.0);
}
