#version 330

in vec3 normalWorldspace;
out vec4 color;

void main(void) {
    color = vec4(normalWorldspace * 0.5 + 0.5, 1.0);
}
