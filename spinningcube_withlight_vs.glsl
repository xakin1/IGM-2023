#version 130

in vec4 v_pos;

uniform mat4 mv_matrix;
uniform mat4 proj_matrix;

void main() {
    gl_Position = proj_matrix * mv_matrix * v_pos;
}