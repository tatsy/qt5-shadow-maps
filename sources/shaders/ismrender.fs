#version 330

#define MAX_VPLs 32

uniform sampler2D ismTexture;
uniform sampler2D accumTexture;

uniform vec3 posVPL[MAX_VPLs];
uniform vec3 nrmVPL[MAX_VPLs];
uniform vec3 albVPL[MAX_VPLs];
uniform mat4 mvVPL[MAX_VPLs];

uniform int currentRow;
uniform int ismRows;
uniform int ismCols;
uniform float maxDepth;

in vec3 vertexWorldspace;
in vec3 normalWorldspace;
in vec3 lightWorldspace;
in vec3 vertexColor;
in vec4 vertexScreenspace;

out vec4 color;

float Pi = 4.0 * atan(1.0);

vec3 sphericalMap(vec3 posCam) {
    vec3 pos = posCam / maxDepth;
    float pz = length(pos);
    if (pos.z > 0.0) {
        pz = -1.0;
    }

    pos = normalize(pos);
    float theta = acos(-pos.z);
    if (theta > Pi * 0.5) {
        theta = Pi - theta;
    }

    float len = sqrt(pos.x * pos.x + pos.y * pos.y);
    return vec3(pos.xy / len * theta / (Pi * 0.5), pz);
}

vec3 reflectiveSM(vec3 V, vec3 N, vec3 Vp, vec3 Np, vec3 Phi) {
    vec3 ret = vec3(0.0, 0.0, 0.0);
    return Phi * max(0.0, dot(Np, V - Vp)) * max(0.0, dot(N, Vp - V)) / pow(length(V - Vp), 4.0);
}

vec3 calcIndirect(vec3 V, vec3 N) {
    float bias = 0.05;
    vec3 indirect = vec3(0.0, 0.0, 0.0);
    for (int j = 0; j < MAX_VPLs; j++) {
        vec4 posCam = mvVPL[j] * vec4(vertexWorldspace, 1.0);
        posCam /= posCam.w;
        vec3 posSph = sphericalMap(posCam.xyz);

        vec2 texCoord = posSph.xy * 0.5 + 0.5;
        texCoord.x = (texCoord.x + j) / ismCols;
        texCoord.y = (texCoord.y + currentRow) / ismRows;
        float distFromVPL = texture(ismTexture, texCoord).z;

        if (distFromVPL + bias >= posSph.z && posCam.w > 0.0) {
            indirect += reflectiveSM(V, N, posVPL[j], nrmVPL[j], albVPL[j]);
        }
    }
    return 4.0 * Pi * indirect / (ismRows * ismCols);
}

void main(void) {
    vec3 indirect = calcIndirect(vertexWorldspace, normalWorldspace);

    vec2 texCoord = vertexScreenspace.xy / vertexScreenspace.w;
    texCoord = texCoord * 0.5 + 0.5;
    vec3 accum = texture2D(accumTexture, texCoord).xyz;
    color = vec4(accum + indirect, 1.0);
}
