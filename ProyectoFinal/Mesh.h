#pragma once

#include <string>
#include <sstream>
#include <iostream>
#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Shader.h"

using namespace std;

struct Vertex
{
    glm::vec3 Position;  // Posición del vértice
    glm::vec3 Normal;    // Normal
    glm::vec2 TexCoords; // Coordenadas de textura
};

struct Texture
{
    GLuint id;
    string type; // "texture_diffuse" o "texture_specular"
    aiString path;
};

class Mesh
{
public:
    // Datos del mesh
    vector<Vertex>  vertices;
    vector<GLuint>  indices;
    vector<Texture> textures;

    // Constructor
    Mesh(vector<Vertex> vertices, vector<GLuint> indices, vector<Texture> textures)
    {
        this->vertices = vertices;
        this->indices = indices;
        this->textures = textures;

        // Configura buffers y atributos
        this->setupMesh();
    }

    // Renderiza el mesh con el shader dado
    void Draw(Shader shader)
    {
        GLuint diffuseNr = 1;
        GLuint specularNr = 1;

        // Vincula texturas
        for (GLuint i = 0; i < this->textures.size(); i++)
        {
            glActiveTexture(GL_TEXTURE0 + i);

            stringstream ss;
            string number;
            string name = this->textures[i].type;

            if (name == "texture_diffuse")
            {
                ss << diffuseNr++;
            }
            else if (name == "texture_specular")
            {
                ss << specularNr++;
            }

            number = ss.str();

            // Nombre esperado en el shader: texture_diffuse1, texture_specular1, etc.
            string uniformName = name + number;
            glUniform1i(glGetUniformLocation(shader.Program, uniformName.c_str()), i);
            glBindTexture(GL_TEXTURE_2D, this->textures[i].id);
        }

        // Shininess por defecto (si no lo sobreescribes afuera)
        glUniform1f(glGetUniformLocation(shader.Program, "material.shininess"), 16.0f);

        // Dibuja
        glBindVertexArray(this->VAO);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(this->indices.size()), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        // Limpia unidades de textura
        for (GLuint i = 0; i < this->textures.size(); i++)
        {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
    }

private:
    // Render data
    GLuint VAO, VBO, EBO;

    // Inicializa VAO / VBO / EBO
    void setupMesh()
    {
        glGenVertexArrays(1, &this->VAO);
        glGenBuffers(1, &this->VBO);
        glGenBuffers(1, &this->EBO);

        glBindVertexArray(this->VAO);

        // Vértices
        glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
        glBufferData(GL_ARRAY_BUFFER,
            this->vertices.size() * sizeof(Vertex),
            &this->vertices[0],
            GL_STATIC_DRAW);

        // Índices
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
            this->indices.size() * sizeof(GLuint),
            &this->indices[0],
            GL_STATIC_DRAW);

        // Atributos de vértice:
        // layout (location = 0) -> Position
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0,
            3,
            GL_FLOAT,
            GL_FALSE,
            sizeof(Vertex),
            (GLvoid*)0);

        // layout (location = 1) -> Normal
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1,
            3,
            GL_FLOAT,
            GL_FALSE,
            sizeof(Vertex),
            (GLvoid*)offsetof(Vertex, Normal));

        // layout (location = 2) -> TexCoords
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2,
            2,
            GL_FLOAT,
            GL_FALSE,
            sizeof(Vertex),
            (GLvoid*)offsetof(Vertex, TexCoords));

        glBindVertexArray(0);
    }
};
