#version 330 core

out vec4 frag_col;

in vec3 normal;
in vec3 frag_3Dpos;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct Light {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Material material;
uniform Light light;
uniform vec3 view_pos;

void main() {
    // ambient
    vec3 ambient = light.ambient * material.ambient;

    vec3 light_dir  = normalize(light.position - frag_3Dpos);
    // diffuse 
    float diff = max(dot(normal, light_dir), 0.0);
    vec3 diffuse = light.diffuse * diff * material.diffuse;

    // // specular
    vec3 view_dir = normalize(view_pos - frag_3Dpos);
    vec3 reflect_dir = reflect(-light_dir, normal);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * material.specular;

    vec3 result = ambient + diffuse + specular;
    frag_col = vec4(result, 1.0);
}