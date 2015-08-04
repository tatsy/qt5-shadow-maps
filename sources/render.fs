#version 330

uniform sampler2D shadowmap;

uniform vec3 cameraPosition;
uniform int receiveShadow;

in vec3 vertexCameraspace;
in vec3 normalCameraspace;
in vec3 lightCameraspace;
in vec3 vertexColor;

in vec4 shadowCoord;

out vec4 color;

vec4 diffuseColor = vec4(0.7, 0.7, 0.7, 1.0);
vec4 specularColor = vec4(1.0, 1.0, 1.0, 1.0);
vec4 ambientColor = vec4(0.1, 0.1, 0.1, 1.0);
float lightness = 64.0;

void main(void) {
  vec3 V = normalize(cameraPosition - vertexCameraspace);
  vec3 N = normalize(normalCameraspace);
  vec3 L = normalize(lightCameraspace - vertexCameraspace);
  vec3 H = normalize(L + V);

  float NdotL = dot(N, L);
  float NdotH = dot(N, H);

  vec4 diffuse = diffuseColor * vec4(max(0.0, NdotL));
  vec4 specular = specularColor * vec4(pow(max(0.0, NdotH), lightness));
  vec4 ambient = ambientColor;

  float bias = 0.005;
  vec4 coord = shadowCoord / shadowCoord.w;
  float distFromLight = texture2D(shadowmap, coord.xy).z + bias;

  float shadow = 1.0;
  if (receiveShadow != 0 && shadowCoord.w > 0.0) {
    shadow = distFromLight < coord.z ? 0.5 : 1.0;
  }

  color = vec4(vertexColor, 1.0) * (diffuse + specular + ambient) * shadow;
}
