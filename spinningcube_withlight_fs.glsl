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
uniform Light light2;

uniform vec3 view_pos;

void main() {
    // ambient
    vec3 ambient = light.ambient * material.ambient;

    vec3 light_dir  = normalize(light.position - frag_3Dpos);
    // diffuse 
    float diff = max(dot(normal, light_dir), 0.0);
    vec3 diffuse = light.diffuse * (diff * material.diffuse);

    // specular
    vec3 view_dir = normalize(view_pos - frag_3Dpos);
    vec3 reflect_dir = reflect(-light_dir, normal);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);
    vec3 specular = light.specular * (spec * material.specular);


    // ambient
    vec3 ambient2 = light2.ambient * material.ambient;

    vec3 light2_dir  = normalize(light2.position - frag_3Dpos);
    // diffuse 
    float diff2 = max(dot(normal, light2_dir), 0.0);
    vec3 diffuse2 = light2.diffuse * (diff2 * material.diffuse);

    // specular
    vec3 view_dir2 = normalize(view_pos - frag_3Dpos);
    vec3 reflect_dir2 = reflect(-light2_dir, normal);
    float spec2 = pow(max(dot(view_dir2, reflect_dir2), 0.0), material.shininess);
    vec3 specular2 = light2.specular * (spec2 * material.specular);

    vec3 result = ambient + diffuse + specular;
    //sumamos la segunda luz
    result += ambient2 + diffuse2 + specular2;
    frag_col = vec4(result, 1.0);
}