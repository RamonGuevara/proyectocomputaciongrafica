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

GLint TextureFromFile(const char* path, string directory);

class Model
{
public:
    Model(GLchar* path)
    {
        std::string realPath(path);

        // Normalizar separadores
        for (char& c : realPath)
        {
            if (c == '\\') c = '/';
        }

        // Si no empieza con "Models/", lo prefixeamos
        const std::string modelsPrefix = "Models/";
        if (realPath.rfind(modelsPrefix, 0) != 0)
        {
            realPath = modelsPrefix + realPath;
        }

        this->loadModel(realPath);
    }

    // Dibuja todas las meshes del modelo
    void Draw(Shader shader)
    {
        for (GLuint i = 0; i < this->meshes.size(); i++)
        {
            this->meshes[i].Draw(shader);
        }
    }

private:
    vector<Mesh> meshes;
    string directory;                // Carpeta donde está el .obj
    vector<Texture> textures_loaded; // Cache de texturas para evitar duplicados

    void loadModel(const string& path)
    {
        Assimp::Importer importer;

        const aiScene* scene = importer.ReadFile(
            path,
            aiProcess_Triangulate |
            aiProcess_FlipUVs
        );

        if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
            return;
        }

        // Carpeta base del .obj (ej: "Models" o "Models/Puerta")
        size_t slashPos = path.find_last_of("/\\");
        if (slashPos != string::npos)
            this->directory = path.substr(0, slashPos);
        else
            this->directory = ".";

        this->processNode(scene->mRootNode, scene);
    }

    void processNode(aiNode* node, const aiScene* scene)
    {
        // Procesar todas las meshes de este nodo
        for (GLuint i = 0; i < node->mNumMeshes; i++)
        {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            this->meshes.push_back(this->processMesh(mesh, scene));
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

                textures.push_back(texture);
                textures_loaded.push_back(texture);
            }
        }

        return textures;
    }
};

GLint TextureFromFile(const char* path, string /*directory*/)
{
    if (!path || !*path)
    {
        std::cout << "ERROR::TEXTURE_LOADING::EMPTY_PATH" << std::endl;
        return 0;
    }

    std::string p(path);

    // Normalizar '\' a '/'
    for (char& c : p)
    {
        if (c == '\\')
            c = '/';
    }

    // Quitar espacios alrededor
    while (!p.empty() && (p.front() == ' ' || p.front() == '\t'))
        p.erase(p.begin());
    while (!p.empty() && (p.back() == ' ' || p.back() == '\t' || p.back() == '\r' || p.back() == '\n'))
        p.pop_back();

    if (p.empty())
    {
        std::cout << "ERROR::TEXTURE_LOADING::EMPTY_PATH_AFTER_TRIM" << std::endl;
        return 0;
    }

    std::string fullPath;

    const std::string texPrefix = "Textures/";
    if (p.rfind(texPrefix, 0) == 0)
    {
        fullPath = p;
    }
    else
    {
        // Si no, asumimos que es relativo a la carpeta Textures/
        // y concatenamos:
        //   "Textures/" + p
        fullPath = texPrefix + p;
    }

    GLuint textureID;
    glGenTextures(1, &textureID);

    int width, height, channels;
    unsigned char* image = SOIL_load_image(fullPath.c_str(), &width, &height, &channels, SOIL_LOAD_AUTO);

    if (!image)
    {
        std::cout << "ERROR::TEXTURE_LOADING::FAILED " << fullPath
            << " (from '" << p << "')" << std::endl;
        return 0;
    }

    GLenum format = GL_RGB;
    if (channels == 1)       format = GL_RED;
    else if (channels == 3)  format = GL_RGB;
    else if (channels == 4)  format = GL_RGBA;

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
