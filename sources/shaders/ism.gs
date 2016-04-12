#version 330
#extension GL_EXT_geometry_shader4 : enable

layout(triangles) in;
//layout(points, max_vertices=64) out;
layout(triangle_strip, max_vertices=192) out;

uniform int smRows;
uniform int smCols;
uniform mat4 mvpVPL[64];

out float depth;

void main(void) {
    float scaleX = 1.0 / smCols;
    float scaleY = 1.0 / smRows;
    for (int i = 0; i < smRows; i++) {
        for (int j = 0; j < smCols; j++) {
            int index = i * smCols + j;
            for (int k = 0; k < 3; k++) {
                gl_Position = mvpVPL[index] * gl_PositionIn[k];
                gl_Position.x = scaleX * gl_Position.x + 2.0 * scaleX * (j + 0.5) - 1.0;
                gl_Position.y = scaleY * gl_Position.y + 2.0 * scaleY * (i + 0.5) - 1.0;
                depth = gl_Position.z;
                EmitVertex();
            }
            EndPrimitive();
        }
    }
}
