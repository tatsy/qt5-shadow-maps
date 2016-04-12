#version 330

in float depth;
out vec4 color;

void main(void) {
    color = vec4(depth, depth, depth, 1.0);
}
