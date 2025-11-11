#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <cstring>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "SOIL2/SOIL2.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Mesh.h"
#include "Shader.h"

using namespace std;

GLint TextureFromFile(const char* path, const std::string& directory);
GLuint LoadDefaultTexture();

class Model
{
public:
    Model(GLchar* path)
    {
        std::string realPath(path);

        // Normalizar separadores
        for (char& c : realPath)
            if (c == '\\') c = '/';

        // Si no empieza con "Models/", lo prefixeamos
        const std::string modelsPrefix = "Models/";
        if (realPath.rfind(modelsPrefix, 0) != 0)
            realPath = modelsPrefix + realPath;

        if (!this->loadModel(realPath))
        {
            std::cout << "Error al cargar el modelo: " << realPath << std::endl;
        }
    }

    void Draw(Shader shader)
    {
        for (GLuint i = 0; i < this->meshes.size(); i++)
            this->meshes[i].Draw(shader);
    }

private:
    vector<Mesh> meshes;
    string directory;                // Carpeta donde está el .obj
    vector<Texture> textures_loaded; // Cache de texturas para evitar duplicados

    bool loadModel(const string& path)
    {
        Assimp::Importer importer;

        const aiScene* scene = importer.ReadFile(
            path,
            aiProcess_Triangulate |
            aiProcess_FlipUVs
        );

        if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            std::cout
                << "Error al cargar el modelo: " << path
                << " - " << importer.GetErrorString()
                << std::endl;
            return false;
        }

        // Carpeta base del .obj (ej: "Models" o "Models/Puerta")
        size_t slashPos = path.find_last_of("/\\");
        if (slashPos != string::npos)
            this->directory = path.substr(0, slashPos);
        else
            this->directory = ".";

        this->processNode(scene->mRootNode, scene);
        return true;
    }

    void processNode(aiNode* node, const aiScene* scene)
    {
        // Procesar todas las meshes de este nodo
        for (GLuint i = 0; i < node->mNumMeshes; i++)
        {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            Mesh m = this->processMesh(mesh, scene);

            // Si por alguna razón viene vacía, la ignoramos
            if (!m.vertices.empty() && !m.indices.empty())
            {
                this->meshes.push_back(m);
            }
            else
            {
                std::cout << "Advertencia: mesh vacía/ inválida omitida." << std::endl;
            }
        }

        // Recorrer hijos
        for (GLuint i = 0; i < node->mNumChildren; i++)
        {
            this->processNode(node->mChildren[i], scene);
        }
    }

    Mesh processMesh(aiMesh* mesh, const aiScene* scene)
    {
        vector<Vertex> vertices;
        vector<GLuint> indices;
        vector<Texture> textures;

        // --- VÉRTICES ---
        for (GLuint i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex;
            glm::vec3 vector;

            // Posición
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.Position = vector;

            // Normales
            if (mesh->HasNormals())
            {
                vector.x = mesh->mNormals[i].x;
                vector.y = mesh->mNormals[i].y;
                vector.z = mesh->mNormals[i].z;
                vertex.Normal = vector;
            }
            else
            {
                vertex.Normal = glm::vec3(0.0f);
            }

            // TexCoords
            if (mesh->mTextureCoords[0])
            {
                glm::vec2 tex;
                tex.x = mesh->mTextureCoords[0][i].x;
                tex.y = mesh->mTextureCoords[0][i].y;
                vertex.TexCoords = tex;
            }
            else
            {
                vertex.TexCoords = glm::vec2(0.0f);
            }

            vertices.push_back(vertex);
        }

        // --- ÍNDICES ---
        for (GLuint i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            for (GLuint j = 0; j < face.mNumIndices; j++)
            {
                indices.push_back(face.mIndices[j]);
            }
        }

        // --- TEXTURAS DEL MATERIAL ---
        if (mesh->mMaterialIndex >= 0)
        {
            aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

            // Difusas
            vector<Texture> diffuseMaps = this->loadMaterialTextures(
                material, aiTextureType_DIFFUSE, "texture_diffuse"
            );
            textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

            // Especulares
            vector<Texture> specularMaps = this->loadMaterialTextures(
                material, aiTextureType_SPECULAR, "texture_specular"
            );
            textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
        }

        // Si no hubo texturas en el material, usar la default
        if (textures.empty())
        {
            Texture t;
            t.id = LoadDefaultTexture();
            t.type = "texture_diffuse";
            t.path = aiString("plain.png");
            textures.push_back(t);
        }

        return Mesh(vertices, indices, textures);
    }

    vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName)
    {
        vector<Texture> textures;

        for (GLuint i = 0; i < mat->GetTextureCount(type); i++)
        {
            aiString str;
            mat->GetTexture(type, i, &str);

            // Ver si ya la cargamos antes (comparando la cadena original del .mtl)
            bool skip = false;
            for (GLuint j = 0; j < textures_loaded.size(); j++)
            {
                if (std::strcmp(textures_loaded[j].path.C_Str(), str.C_Str()) == 0)
                {
                    textures.push_back(textures_loaded[j]);
                    skip = true;
                    break;
                }
            }

            if (!skip)
            {
                Texture texture;
                texture.id = TextureFromFile(str.C_Str(), this->directory);
                texture.type = typeName;
                texture.path = str;

                if (texture.id == 0)
                {
                    std::cout
                        << "Error al cargar la textura: " << str.C_Str()
                        << ", usando Textures/plain.png" << std::endl;
                    texture.id = LoadDefaultTexture();
                    texture.path = aiString("plain.png");
                }

                textures.push_back(texture);
                textures_loaded.push_back(texture);
            }
        }

        return textures;
    }
};

// ---------------- IMPLEMENTACIÓN TEXTURAS ----------------

GLint TextureFromFile(const char* path, const std::string& directory)
{
    if (!path || !*path)
    {
        std::cout << "Error al cargar la textura: [ruta vacía], usando Textures/plain.png" << std::endl;
        return LoadDefaultTexture();
    }

    std::string p(path);

    // Normalizar '\' a '/'
    for (char& c : p)
        if (c == '\\') c = '/';

    // Quitar espacios alrededor
    while (!p.empty() && (p.front() == ' ' || p.front() == '\t'))
        p.erase(p.begin());
    while (!p.empty() && (p.back() == ' ' || p.back() == '\t' || p.back() == '\r' || p.back() == '\n'))
        p.pop_back();

    if (p.empty())
    {
        std::cout << "Error al cargar la textura: [ruta vacía tras limpiar], usando Textures/plain.png" << std::endl;
        return LoadDefaultTexture();
    }

    std::string fullPath;

    // Si ya empieza con Textures/, úsalo directo
    const std::string texPrefix = "Textures/";
    if (p.rfind(texPrefix, 0) == 0)
    {
        fullPath = p;
    }
    else if (!directory.empty())
    {
        // Primero probamos relativo al modelo
        fullPath = directory + "/" + p;
    }
    else
    {
        // Si no hay directorio, probamos en Textures/
        fullPath = texPrefix + p;
    }

    int width, height, channels;
    unsigned char* image = SOIL_load_image(fullPath.c_str(), &width, &height, &channels, SOIL_LOAD_AUTO);

    // Si falla, probamos directamente en Textures/<archivo> como respaldo
    if (!image)
    {
        std::string alt = texPrefix + p;
        if (alt != fullPath)
        {
            image = SOIL_load_image(alt.c_str(), &width, &height, &channels, SOIL_LOAD_AUTO);
            if (image)
                fullPath = alt;
        }
    }

    if (!image)
    {
        std::cout << "Error al cargar la textura: " << p
            << ", usando Textures/plain.png" << std::endl;
        return LoadDefaultTexture();
    }

    GLenum format = GL_RGB;
    if (channels == 1)       format = GL_RED;
    else if (channels == 3)  format = GL_RGB;
    else if (channels == 4)  format = GL_RGBA;

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexImage2D(GL_TEXTURE_2D,
        0,
        format,
        width,
        height,
        0,
        format,
        GL_UNSIGNED_BYTE,
        image);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);
    SOIL_free_image_data(image);

    return textureID;
}

GLuint LoadDefaultTexture()
{
    static GLuint defaultTex = 0;
    if (defaultTex != 0)
        return defaultTex;

    const std::string fallback = "Textures/plain.png";

    int width, height, channels;
    unsigned char* image = SOIL_load_image(fallback.c_str(), &width, &height, &channels, SOIL_LOAD_AUTO);

    if (!image)
    {
        std::cout
            << "Error al cargar la textura: " << fallback
            << ", creando textura blanca por defecto." << std::endl;

        unsigned char white[4] = { 255, 255, 255, 255 };

        glGenTextures(1, &defaultTex);
        glBindTexture(GL_TEXTURE_2D, defaultTex);
        glTexImage2D(GL_TEXTURE_2D,
            0,
            GL_RGBA,
            1, 1,
            0,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            white);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glBindTexture(GL_TEXTURE_2D, 0);
        return defaultTex;
    }

    GLenum format = GL_RGB;
    if (channels == 1)       format = GL_RED;
    else if (channels == 3)  format = GL_RGB;
    else if (channels == 4)  format = GL_RGBA;

    glGenTextures(1, &defaultTex);
    glBindTexture(GL_TEXTURE_2D, defaultTex);

    glTexImage2D(GL_TEXTURE_2D,
        0,
        format,
        width,
        height,
        0,
        format,
        GL_UNSIGNED_BYTE,
        image);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);
    SOIL_free_image_data(image);

    std::cout << "Usando textura por defecto: " << fallback << std::endl;
    return defaultTex;
}
