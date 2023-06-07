#version 330 core

in vec3 v_pos;
in vec3 v_normal;
in vec2 v_texture;

out vec3 frag_3Dpos;
out vec3 normal;
out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normal_matrix;

void main() {
    frag_3Dpos = vec3(model * vec4(v_pos, 1.0));
    normal = normalize(normal_matrix * v_normal);
    gl_Position = projection * view * model * vec4(v_pos, 1.0f);
    TexCoords = v_texture;
}