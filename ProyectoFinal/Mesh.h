#pragma once

#include <vector>
#include <iostream>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include "Shader.h"
#include "Texture.h"
#include "Model.h"

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};

class Mesh
{
public:
    std::vector<Vertex>  vertices;
    std::vector<GLuint>  indices;
    std::vector<Texture> textures;
    GLuint VAO, VBO, EBO;

    Mesh(std::vector<Vertex> vertices,
        std::vector<GLuint> indices,
        std::vector<Texture> textures)
    {
        this->vertices = std::move(vertices);
        this->indices = std::move(indices);
        this->textures = std::move(textures);

        VAO = VBO = EBO = 0;
        setupMesh();
    }

    void Draw(Shader shader)
    {
        // Si la mesh es inválida, no intentamos dibujarla
        if (VAO == 0 || indices.empty())
            return;

        GLuint diffuseNr = 1;
        GLuint specularNr = 1;

        for (GLuint i = 0; i < textures.size(); i++)
        {
            glActiveTexture(GL_TEXTURE0 + i);

            std::string number;
            std::string name = textures[i].type;

            if (name == "texture_diffuse")
                number = std::to_string(diffuseNr++);
            else if (name == "texture_specular")
                number = std::to_string(specularNr++);

            std::string uniformName = name + number;
            GLint loc = glGetUniformLocation(shader.Program, uniformName.c_str());
            if (loc != -1)
                glUniform1i(loc, i);

            glBindTexture(GL_TEXTURE_2D, textures[i].id);
        }

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        glActiveTexture(GL_TEXTURE0);
    }

private:
    void setupMesh()
    {
        //  Punto crítico: si no hay datos, NO creamos buffers.
        if (vertices.empty() || indices.empty())
        {
            std::cout << "Mesh::setupMesh -> Mesh vacia (vertices="
                << vertices.size() << ", indices=" << indices.size()
                << "), se omite VAO." << std::endl;
            VAO = VBO = EBO = 0;
            return;
        }

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER,
            vertices.size() * sizeof(Vertex),
            vertices.data(),                // seguro: no está vacío
            GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
            indices.size() * sizeof(GLuint),
            indices.data(),                 // seguro
            GL_STATIC_DRAW);

        // layout posición
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
            sizeof(Vertex), (GLvoid*)offsetof(Vertex, Position));

        // layout normal
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
            sizeof(Vertex), (GLvoid*)offsetof(Vertex, Normal));

        // layout texcoords
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE,
            sizeof(Vertex), (GLvoid*)offsetof(Vertex, TexCoords));

        glBindVertexArray(0);
    }
};
