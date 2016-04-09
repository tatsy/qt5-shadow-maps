#version 330

int MODE_SM  = 0;
int MODE_RSM = 1;

uniform sampler2D depthMap;
uniform sampler2D normalMap;
uniform sampler2D positionMap;
uniform sampler2D albedoMap;

uniform vec3 cameraPosition;
uniform int receiveShadow;
uniform int mode;

uniform int nSamples;
uniform vec2 delta[256];

in vec3 vertexWorldspace;
in vec3 normalWorldspace;

in vec3 vertexCameraspace;
in vec3 normalCameraspace;
in vec3 lightCameraspace;
in vec3 vertexColor;

in vec4 shadowCoord;

out vec4 color;

vec4 diffuseColor = vec4(0.7, 0.7, 0.7, 1.0);
vec4 specularColor = vec4(1.0, 1.0, 1.0, 1.0);
vec4 ambientColor = vec4(0.0, 0.0, 0.0, 1.0);

float lightness = 64.0;

vec3 reflectiveSM(vec3 V, vec3 N) {
    vec3 ret = vec3(0.0, 0.0, 0.0);
    for (int i = 0; i < nSamples; i++) {
        vec2 coord = shadowCoord.st / shadowCoord.w + delta[i];
        vec3 Np  = texture2D(normalMap, coord.xy).xyz;
        vec3 Vp  = texture2D(positionMap, coord.xy).xyz * 10.0 - vec3(5.0, 5.0, 5.0);
        vec3 Phi = texture2D(albedoMap, coord.xy).xyz;
        ret += Phi * max(0.0, dot(Np, V - Vp)) * max(0.0, dot(N, Vp - V)) / pow(length(V - Vp), 4.0);
    }
    return ret;
}

void main(void) {
  vec3 V = -normalize(vertexCameraspace);
  vec3 N =  normalize(normalCameraspace);

  vec3 L = normalize(lightCameraspace - vertexCameraspace);
  vec3 H = normalize(L + V);

  float NdotL = dot(N, L);
  float NdotH = dot(N, H);

  vec4 diffuse = diffuseColor * vec4(max(0.0, NdotL));
  vec4 specular = specularColor * vec4(pow(max(0.0, NdotH), lightness));
  vec4 ambient = ambientColor;

  float bias = 0.005;
  vec4 coord = shadowCoord / shadowCoord.w;
  float distFromLight = texture2D(depthMap, coord.xy).z + bias;

  float shadow = 1.0;
  if (receiveShadow != 0 && shadowCoord.w > 0.0) {
    shadow = distFromLight < coord.z ? 0.5 : 1.0;
  }

  if (mode == MODE_SM) {
      color = vec4(vertexColor, 1.0) * (diffuse + specular + ambient) * shadow;
  } else if (mode == MODE_RSM) {
      color = vec4(vertexColor, 1.0) * (diffuse + specular + ambient) * shadow;

      if (receiveShadow == 0) {
          vec3 rsm = reflectiveSM(vertexWorldspace, normalWorldspace);
          color += vec4(rsm, 1.0);
      }
  }
}
