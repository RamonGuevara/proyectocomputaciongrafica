#version 330 core

layout (location = 0) in vec3 position;

out vec3 TexCoords;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    // Eliminamos la traslación de la vista para que el skybox siga la cámara
    mat4 rotView = mat4(mat3(view));
    vec4 pos = projection * rotView * vec4(position, 1.0);

    // pos.xyww mantiene el skybox siempre al fondo
    gl_Position = pos.xyww;
    TexCoords = position;
}
