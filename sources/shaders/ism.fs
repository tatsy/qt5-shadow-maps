#version 330

in float depth;
out vec4 color;

void main(void) {
    if (depth < 0.0) discard;
    color = vec4(depth, depth, depth, 1.0);
}
