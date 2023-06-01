#version 130

uniform vec3 light_pos;
uniform vec3 light_ambient;
uniform vec3 light_diffuse;
uniform vec3 light_specular;

uniform vec3 material_ambient;
uniform vec3 material_diffuse;
uniform vec3 material_specular;
uniform float material_shininess;

in vec3 FragPos;
in vec3 Normal;

out vec4 FragColor;

void main() {

    vec3 texture_color = vec3(1.0f, 0.0f, 0.0f);

    // Ambient light
    vec3 ambient = light_ambient * material_ambient;

    // Diffuse light
    vec3 lightDir = normalize(light_pos - FragPos);
    float diff = max(dot(Normal, lightDir), 0.0);
    vec3 diffuse = light_diffuse * (diff * material_diffuse * texture_color);

    // Specular light
    vec3 viewDir = normalize(-FragPos);
    vec3 reflectDir = reflect(-lightDir, Normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material_shininess);
    vec3 specular = light_specular * (spec * material_specular);

    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);

}