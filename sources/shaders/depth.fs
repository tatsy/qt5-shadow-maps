#version 330

in float depth;
out vec4 color;

vec3 depthToRGB(float d) {
    float r = d;
    float g = fract(r * 255.0);
    float b = fract(g * 255.0);
    float coef = 1.0 / 255.0;
    r -= g * coef;
    g -= b * coef;
    return vec3(r, g, b);
}

void main(void) {
    color = vec4(depthToRGB(depth), 1.0);
}
