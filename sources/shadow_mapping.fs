#version 330

int MODE_DEPTH   = 0;
int MODE_NORMAL  = 1;
int MODE_POSITON = 2;
int MODE_COLOR   = 3;

uniform int mode;

in vec3 vertexWorldspace;
in vec3 normalWorldspace;
in vec3 vertexColor;

out vec4 color;

void main(void) {
    if (mode == MODE_DEPTH) {
        float depth = gl_FragCoord.z;
        color = vec4(depth, depth, depth, 1.0);
    } else if (mode == MODE_NORMAL) {
        color = vec4(normalWorldspace, 1.0);
    } else if (mode == MODE_POSITON) {
        vec3 V = (vertexWorldspace + vec3(5.0, 5.0, 5.0)) / 10.0;
        color = vec4(V, 1.0);
    } else if (mode == MODE_COLOR) {
        color = vec4(vertexColor, 1.0);
    }
}
