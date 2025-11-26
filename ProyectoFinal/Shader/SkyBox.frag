#version 330 core

out vec4 FragColor;
in vec3 TexCoords;

uniform samplerCube skyboxDay;
uniform samplerCube skyboxNight;
uniform float blendFactor;

void main()
{
    vec4 dayColor   = texture(skyboxDay, TexCoords);
    vec4 nightColor = texture(skyboxNight, TexCoords);

    // blendFactor = 1.0 -> noche
    // blendFactor = 0.0 -> día
    FragColor = mix(dayColor, nightColor, blendFactor);
}
