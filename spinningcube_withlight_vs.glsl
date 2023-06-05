#version 330 core

in vec3 v_pos;
in vec3 v_normal;

out vec3 frag_3Dpos;
out vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normal_matrix;

void main() {
    gl_Position = projection * view * model * vec4(v_pos, 1.0f);
    frag_3Dpos = vec3(model * vec4(v_pos, 1.0));
    normal = normalize(normal_matrix * v_normal);
}