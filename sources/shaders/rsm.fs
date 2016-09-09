#version 330

in vec3 f_posWorld;
in vec3 f_nrmWorld;
in vec4 f_posScreen;
in vec4 f_color;

layout(location = 0) out vec4 out_depth;
layout(location = 1) out vec4 out_position;
layout(location = 2) out vec4 out_normal;
layout(location = 3) out vec4 out_color;

void main(void) {
    float depth = f_posScreen.z / f_posScreen.w;
    out_depth = vec4(depth, depth, depth, 1.0);

    out_position = vec4(f_posWorld, 1.0);
    out_normal = vec4(normalize(f_nrmWorld) * 0.5 + 0.5, 1.0);
    out_color = vec4(f_color.rgb, 1.0);
}
