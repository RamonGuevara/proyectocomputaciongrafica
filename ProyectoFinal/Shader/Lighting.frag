#version 330 core

#define NUMBER_OF_POINT_LIGHTS 1

struct DirLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct Material {
    float shininess;
};

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

out vec4 color;

uniform vec3 viewPos;

uniform DirLight  dirLight;
uniform PointLight pointLights[NUMBER_OF_POINT_LIGHTS];
uniform SpotLight spotLight;
uniform Material  material;

// Texturas controladas desde Mesh / main
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;

// 0 = normal, 1 = aplicar test de alpha y descartar
uniform int transparency;

// Prototipos
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{
    vec3 norm    = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    // Luz direccional
    vec3 result = CalcDirLight(dirLight, norm, viewDir);

    // Luces puntuales
    for (int i = 0; i < NUMBER_OF_POINT_LIGHTS; i++)
        result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);

    // Spotlight (linterna cámara)
    result += CalcSpotLight(spotLight, norm, FragPos, viewDir);

    // Color base de la textura difusa (por si queremos alpha)
    vec4 texColor = texture(texture_diffuse1, TexCoords);

    if (transparency == 1) {
        // Si la textura tiene canal alpha y es muy bajo, descartamos
        if (texColor.a < 0.1)
            discard;
        color = vec4(result, texColor.a);
    } else {
        color = vec4(result, 1.0);
    }
}

// ===============================
// Funciones de iluminación
// ===============================

// Direccional
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);

    // Difusa
    float diff = max(dot(normal, lightDir), 0.0);

    // Especular (Blinn-Phong opcional, aquí Phong clásico)
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    vec3 albedo    = texture(texture_diffuse1, TexCoords).rgb;
    vec3 specColor = texture(texture_specular1, TexCoords).rgb;

    vec3 ambient  = light.ambient  * albedo;
    vec3 diffuse  = light.diffuse  * diff * albedo;
    vec3 specular = light.specular * spec * specColor;

    return ambient + diffuse + specular;
}

// Puntual
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);

    float diff = max(dot(normal, lightDir), 0.0);

    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    float distance    = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant +
                               light.linear * distance +
                               light.quadratic * (distance * distance));

    vec3 albedo    = texture(texture_diffuse1, TexCoords).rgb;
    vec3 specColor = texture(texture_specular1, TexCoords).rgb;

    vec3 ambient  = light.ambient  * albedo;
    vec3 diffuse  = light.diffuse  * diff * albedo;
    vec3 specular = light.specular * spec * specColor;

    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;

    return ambient + diffuse + specular;
}

// Spotlight
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);

    float diff = max(dot(normal, lightDir), 0.0);

    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    float distance    = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant +
                               light.linear * distance +
                               light.quadratic * (distance * distance));

    float theta   = dot(lightDir, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    vec3 albedo    = texture(texture_diffuse1, TexCoords).rgb;
    vec3 specColor = texture(texture_specular1, TexCoords).rgb;

    vec3 ambient  = light.ambient  * albedo;
    vec3 diffuse  = light.diffuse  * diff * albedo;
    vec3 specular = light.specular * spec * specColor;

    ambient  *= attenuation * intensity;
    diffuse  *= attenuation * intensity;
    specular *= attenuation * intensity;

    return ambient + diffuse + specular;
}
