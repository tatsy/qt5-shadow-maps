#version 330
#extension GL_EXT_geometry_shader4 : enable

layout(points) in;
layout(points, max_vertices=32) out;

uniform int currentRow;
uniform int ismRows;
uniform int ismCols;
uniform mat4 mvVPL[32];
uniform float maxDepth;

out float depth;

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

void main(void) {
    float scaleX = 1.0 / ismCols;
    float scaleY = 1.0 / ismRows;
    for (int j = 0; j < ismCols; j++) {
        vec4 temp = mvVPL[j] * gl_PositionIn[0];
        vec3 posCam = temp.xyz / temp.w;
        gl_Position = vec4(sphericalMap(posCam), 1.0);
        if (length(gl_Position.xy) > 0.95) continue;
        gl_Position.x = scaleX * gl_Position.x + scaleX * (2.0 * j + 1.0) - 1.0;
        gl_Position.y = scaleY * gl_Position.y + scaleY * (2.0 * (ismRows - currentRow - 1) + 1.0) - 1.0;
        depth = gl_Position.z;
        gl_PointSize = (1.0 - depth) * 8.0;
        EmitVertex();
        EndPrimitive();
    }
}
