#version 330 core

#define NUMBER_OF_POINT_LIGHTS 3

struct Material
{
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

struct DirLight
{
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight
{
    vec3 position;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct SpotLight
{
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

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

out vec4 color;

uniform vec3 viewPos;
uniform DirLight dirLight;
uniform PointLight pointLights[NUMBER_OF_POINT_LIGHTS];
uniform SpotLight spotLight;
uniform Material material;
uniform int transparency;

// === agregado: control fino de opacidad ===
uniform float alpha;   // 0.0 totalmente transparente, 1.0 opaco

// === agregado: color plano opcional (casa, pyraminx, etc.) ===
uniform bool useFlatColor;   // true => ignora textura difusa y usa flatColor
uniform vec3 flatColor;

// Function prototypes
vec3 CalcDirLight( DirLight light, vec3 normal, vec3 viewDir, vec3 baseColor );
vec3 CalcPointLight( PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 baseColor );
vec3 CalcSpotLight( SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 baseColor );

void main()
{
    // Propiedades básicas
    vec3 norm    = normalize( Normal );
    vec3 viewDir = normalize( viewPos - FragPos );

    // Color base: de textura o plano
    vec3 texDiffuse = texture( material.diffuse, TexCoords ).rgb;
    vec3 baseColor  = useFlatColor ? flatColor : texDiffuse;

    // Directional light
    vec3 result = CalcDirLight( dirLight, norm, viewDir, baseColor );

    // Point lights
    for ( int i = 0; i < NUMBER_OF_POINT_LIGHTS; i++ )
    {
        result += CalcPointLight( pointLights[i], norm, FragPos, viewDir, baseColor );
    }

    // Spot light
    result += CalcSpotLight( spotLight, norm, FragPos, viewDir, baseColor );

    // Canal alpha: de la textura difusa, escalado por uniform alpha
    float texA = texture( material.diffuse, TexCoords ).a;
    float finalAlpha = texA * alpha;

    color = vec4( result, finalAlpha );

    // Descartar fragmentos casi transparentes si transparency == 1
    if ( color.a < 0.1 && transparency == 1 )
        discard;
}

// ======================================================================
// Calculates the color when using a directional light.
// ======================================================================
vec3 CalcDirLight( DirLight light, vec3 normal, vec3 viewDir, vec3 baseColor )
{
    vec3 lightDir = normalize( -light.direction );

    // Diffuse shading
    float diff = max( dot( normal, lightDir ), 0.0 );

    // Specular shading
    vec3 reflectDir = reflect( -lightDir, normal );
    float spec = pow( max( dot( viewDir, reflectDir ), 0.0 ), material.shininess );

    // Textura de especular (sí se sigue tomando del mapa specular)
    vec3 specTex = texture( material.specular, TexCoords ).rgb;

    // Combine results
    vec3 ambient  = light.ambient  * baseColor;
    vec3 diffuse  = light.diffuse  * diff * baseColor;
    vec3 specular = light.specular * spec * specTex;

    return ( ambient + diffuse + specular );
}

// ======================================================================
// Calculates the color when using a point light.
// ======================================================================
vec3 CalcPointLight( PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 baseColor )
{
    vec3 lightDir = normalize( light.position - fragPos );

    // Diffuse shading
    float diff = max( dot( normal, lightDir ), 0.0 );

    // Specular shading
    vec3 reflectDir = reflect( -lightDir, normal );
    float spec = pow( max( dot( viewDir, reflectDir ), 0.0 ), material.shininess );

    // Attenuation
    float distance    = length( light.position - fragPos );
    float attenuation = 1.0f / ( light.constant
                               + light.linear * distance
                               + light.quadratic * ( distance * distance ) );

    vec3 specTex = texture( material.specular, TexCoords ).rgb;

    // Combine results
    vec3 ambient  = light.ambient  * baseColor;
    vec3 diffuse  = light.diffuse  * diff * baseColor;
    vec3 specular = light.specular * spec * specTex;

    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;

    return ( ambient + diffuse + specular );
}

// ======================================================================
// Calculates the color when using a spot light.
// ======================================================================
vec3 CalcSpotLight( SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 baseColor )
{
    vec3 lightDir = normalize( light.position - fragPos );

    // Diffuse shading
    float diff = max( dot( normal, lightDir ), 0.0 );

    // Specular shading
    vec3 reflectDir = reflect( -lightDir, normal );
    float spec = pow( max( dot( viewDir, reflectDir ), 0.0 ), material.shininess );

    // Attenuation
    float distance    = length( light.position - fragPos );
    float attenuation = 1.0f / ( light.constant
                               + light.linear * distance
                               + light.quadratic * ( distance * distance ) );

    // Spotlight intensity
    float theta    = dot( lightDir, normalize( -light.direction ) );
    float epsilon  = light.cutOff - light.outerCutOff;
    float intensity = clamp( ( theta - light.outerCutOff ) / epsilon, 0.0, 1.0 );

    vec3 specTex = texture( material.specular, TexCoords ).rgb;

    // Combine results
    vec3 ambient  = light.ambient  * baseColor;
    vec3 diffuse  = light.diffuse  * diff * baseColor;
    vec3 specular = light.specular * spec * specTex;

    ambient  *= attenuation * intensity;
    diffuse  *= attenuation * intensity;
    specular *= attenuation * intensity;

    return ( ambient + diffuse + specular );
}
