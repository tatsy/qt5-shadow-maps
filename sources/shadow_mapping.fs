#version 330

out vec4 color;

void main() {
  float depth = gl_FragCoord.z;
  color = vec4(depth, depth, depth, 1.0);
}
