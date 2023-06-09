#version 330 core

out vec4 frag_col;

in vec3 normal;
in vec3 frag_3Dpos;
in vec2 TexCoords;

struct Material {
    sampler2D diffuse;
    sampler2D specular;
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
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));

    vec3 light_dir  = normalize(light.position - frag_3Dpos);
    // diffuse 
    float diff = max(dot(normal, light_dir), 0.0);
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));

    // specular
    vec3 view_dir = normalize(view_pos - frag_3Dpos);
    vec3 reflect_dir = reflect(-light_dir, normal);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));


    // ambient 2
    vec3 ambient2 = light2.ambient * vec3(texture(material.diffuse, TexCoords));

    vec3 light2_dir  = normalize(light2.position - frag_3Dpos);
    // diffuse 2
    float diff2 = max(dot(normal, light2_dir), 0.0);
    vec3 diffuse2 = light2.diffuse * diff2 * vec3(texture(material.diffuse, TexCoords));

    // specular 2
    vec3 view_dir2 = normalize(view_pos - frag_3Dpos);
    vec3 reflect_dir2 = reflect(-light2_dir, normal);
    float spec2 = pow(max(dot(view_dir2, reflect_dir2), 0.0), material.shininess);
    vec3 specular2 = light2.specular * spec2 * vec3(texture(material.specular, TexCoords));

    vec3 result = ambient + diffuse + specular;
    //sumamos la segunda luz
    result += ambient2 + diffuse2 + specular2;
    frag_col = vec4(result, 1.0);
}