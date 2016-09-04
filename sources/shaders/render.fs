#version 330

in vec3 f_posView;
in vec3 f_nrmView;
in vec3 f_lightPos;
in vec3 f_color;

in vec3 f_posWorld;
in vec3 f_nrmWorld;

in vec4 f_posLightSpace;

out vec4 out_color;

uniform sampler2D u_depthMap;
uniform sampler2D u_positionMap;
uniform sampler2D u_normalMap;
uniform sampler2D u_diffuseMap;

uniform sampler1D u_randMap;

uniform int u_nSamples;
uniform float u_sampleRadius;

float Pi = 4.0 * atan(1.0);

void main(void) {
    // Indirect illumination
    vec3 indirect = vec3(0.0, 0.0, 0.0);
    vec2 uvLightSpace = f_posLightSpace.xy / f_posLightSpace.w * 0.5 + 0.5;
    for (int i = 0; i < u_nSamples; i++) {
        float u1 = u_sampleRadius * texture(u_randMap, 0.5 * (i * 2) / u_nSamples).x;
        float u2 = 2.0 * Pi * texture(u_randMap, 0.5 * (i * 2 + 1) / u_nSamples).x;
        vec2 offset = u1 * vec2(cos(u2), sin(u2));
        vec2 uv = uvLightSpace + offset;

        vec3 pos = texture(u_positionMap, uv).xyz;
        vec3 nrm = normalize(texture(u_normalMap, uv).xyz);
        vec3 diff = texture(u_diffuseMap, uv).rgb;

        float dot1 = max(0.0, dot(pos - f_posWorld, normalize(f_nrmWorld)));
        float dot2 = max(0.0, -dot(f_posWorld - pos, nrm));
        float dist = length(pos - f_posWorld);

        indirect += diff * (dot1 * dot2) / (dist * dist * dist * dist);
    }
    indirect = 4.0 * Pi * indirect / u_nSamples;

    // Shadow
    float visibility = 1.0;
    float depth = texture(u_depthMap, uvLightSpace).x;
    float zValue = f_posLightSpace.z / f_posLightSpace.w;
    if (f_posLightSpace.w > 0.0 && depth < zValue - 0.01) {
        visibility = 0.5;
    }

    // Direct illumination
    vec3 V = normalize(-f_posView);
    vec3 N = normalize(f_nrmView);
    vec3 L = normalize(f_lightPos - f_posView);

    float ndotl = max(0.0, dot(N, L));
    out_color.rgb = visibility * (f_color * ndotl + f_color * indirect);
    out_color.a = 1.0;
}
