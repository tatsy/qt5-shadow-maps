#version 330

layout(triangles) in;
layout(triangle_strip, max_vertices=18) out;

in vec3 g_position[];
in vec3 g_normal[];
in vec4 g_color[];

out vec3 f_posWorld;
out vec3 f_nrmWorld;
out vec4 f_posScreen;
out vec4 f_color;

uniform mat4 u_projMat;
uniform mat4 u_mvMat[6];

int cubeX[6] = int[6]( 0, 2, 1, 1, 1, 3 );
int cubeY[6] = int[6]( 1, 1, 0, 2, 1, 1 );

void main(void) {
    float scaleX = 1.0 / 4.0;
    float scaleY = 1.0 / 3.0;
    float aspect = 3.0 / 4.0;

    for (int i = 0; i < 6; i++) {
        mat4 mvpMat = u_projMat * u_mvMat[i];
        for (int k = 0; k < 3; k++) {
            gl_Position = mvpMat * vec4(g_position[k], 1.0);
            gl_Position.x = scaleX * gl_Position.x + scaleX * (2.0 * cubeX[i] + 1.0) - 1.0;
            gl_Position.y = scaleY * gl_Position.y + scaleY * (2.0 * (2 - cubeY[i]) + 1.0) - 1.0;

            f_posWorld = g_position[k];
            f_nrmWorld = g_normal[k];
            f_posScreen = mvpMat * vec4(g_position[k], 1.0);
            f_color = g_color[k];

            EmitVertex();
        }
        EndPrimitive();
    }
}
