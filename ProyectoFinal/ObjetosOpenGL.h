#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

// =====================================================================
//  Clase general para almacenar VAOs, VBOs y número de vértices
// =====================================================================
struct SimpleObjectGL
{
    GLuint VAO = 0;
    GLuint VBO = 0;
    GLuint EBO = 0;
    GLsizei indexCount = 0;

    void Create(const std::vector<float>& verts,
        const std::vector<unsigned int>& inds,
        bool hasIndices = true)
    {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER,
            verts.size() * sizeof(float),
            verts.data(), GL_STATIC_DRAW);

        if (hasIndices)
        {
            glGenBuffers(1, &EBO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                inds.size() * sizeof(unsigned int),
                inds.data(), GL_STATIC_DRAW);
            indexCount = inds.size();
        }
        else indexCount = verts.size() / 8;

        // Posición (0)
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
            8 * sizeof(GLfloat), (void*)0);
        glEnableVertexAttribArray(0);

        // Texcoords (2)
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE,
            8 * sizeof(GLfloat), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);

        // Normales (1)
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
            8 * sizeof(GLfloat), (void*)(5 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
    }

    void Draw(GLint modelLoc, const glm::mat4& model) const
    {
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glBindVertexArray(VAO);

        if (EBO != 0)
            glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
        else
            glDrawArrays(GL_TRIANGLES, 0, indexCount);

        glBindVertexArray(0);
    }
};

// =====================================================================
//  Clase que contiene todos tus objetos OpenGL
// =====================================================================
class ObjetosOpenGL
{
public:
    SimpleObjectGL CasaBody;
    SimpleObjectGL CasaRoof;
    SimpleObjectGL CasaPuerta;
    SimpleObjectGL CasaVentanas[6];
    SimpleObjectGL ArbolTronco[2];
    SimpleObjectGL ArbolCopa[2];

    SimpleObjectGL PyrBase;
    std::vector<SimpleObjectGL> PyrStickers;

    SimpleObjectGL Octaedro;

public:
    // ============================================================
    //  Inicializar TODOS los objetos
    // ============================================================
    void Init()
    {
        CrearCasa();
        CrearPyraminx();
        CrearOctaedro();
    }

    // ============================================================
    //  ===================     CASA     ===========================
    // ============================================================
    void CrearCasa()
    {
        // ---------- 1) Cuerpo de la casa (cubo texturizado) ----------
        {
            std::vector<float> v = {
                // pos               uv        normal
                -1,-0.6, 1, 0,0,   0,0,1,
                 1,-0.6, 1, 1,0,   0,0,1,
                 1, 0.6, 1, 1,1,   0,0,1,
                -1, 0.6, 1, 0,1,   0,0,1,

                -1,-0.6,-1, 0,0,  0,0,-1,
                 1,-0.6,-1, 1,0,  0,0,-1,
                 1, 0.6,-1, 1,1,  0,0,-1,
                -1, 0.6,-1, 0,1,  0,0,-1
            };

            std::vector<unsigned int> idx = {
                0,1,2,  2,3,0,    // frente
                1,5,6,  6,2,1,    // derecha
                5,4,7,  7,6,5,    // atrás
                4,0,3,  3,7,4,    // izquierda
                4,5,1,  1,0,4,    // piso
                3,2,6,  6,7,3     // techo plano
            };
            CasaBody.Create(v, idx);
        }

        // ---------- 2) Techo (pirámide cuadrangular) ----------
        {
            std::vector<float> v = {
                // base
                -1,0, 1, 0,0, 0,-1,0,
                 1,0, 1, 1,0, 0,-1,0,
                 1,0,-1, 1,1, 0,-1,0,
                -1,0,-1, 0,1, 0,-1,0,

                // punta
                0,1,0, 0.5,0.5, 0,1,0
            };
            std::vector<unsigned int> idx = {
                // lados
                0,1,4,
                1,2,4,
                2,3,4,
                3,0,4
            };
            CasaRoof.Create(v, idx);
        }

        // ---------- 3) Puerta ----------
        {
            std::vector<float> v = {
                -0.3,-0.6,1.01, 0,0, 0,0,1,
                 0.3,-0.6,1.01, 1,0, 0,0,1,
                 0.3, 0.0,1.01, 1,1, 0,0,1,
                -0.3, 0.0,1.01, 0,1, 0,0,1
            };
            std::vector<unsigned int> idx = {
                0,1,2, 2,3,0
            };
            CasaPuerta.Create(v, idx);
        }

        // ---------- 4) Ventanas (6 rectángulos: 2 frontales, 2 laterales, 2 traseras) ----------
        for (int i = 0; i < 6; i++)
        {
            std::vector<float> v = {
                -0.25,0.1,1.01, 0,0, 0,0,1,
                 0.25,0.1,1.01, 1,0, 0,0,1,
                 0.25,0.4,1.01, 1,1, 0,0,1,
                -0.25,0.4,1.01, 0,1, 0,0,1
            };
            std::vector<unsigned int> idx = { 0,1,2,2,3,0 };
            CasaVentanas[i].Create(v, idx);
        }

        // ---------- 5) Árboles ----------
        for (int i = 0; i < 2; i++)
        {
            // Tronco (cilindro simplificado en prisma)
            std::vector<float> v = {
                -0.1,-0.7,0, 0,0, 0,0,1,
                 0.1,-0.7,0, 1,0, 0,0,1,
                 0.1, 0.1,0, 1,1, 0,0,1,
                -0.1, 0.1,0, 0,1, 0,0,1
            };
            std::vector<unsigned int> idx = { 0,1,2,2,3,0 };
            ArbolTronco[i].Create(v, idx);

            // Copa (triángulo)
            std::vector<float> v2 = {
                -0.5,0.2,0, 0,0, 0,0,1,
                 0.5,0.2,0, 1,0, 0,0,1,
                 0.0,0.9,0, 0.5,1, 0,0,1
            };
            std::vector<unsigned int> idx2 = { 0,1,2 };
            ArbolCopa[i].Create(v2, idx2);
        }
    }

    // ============================================================
    //  =================  PYRAMINX (Rubik)  =======================
    // ============================================================
    void CrearPyraminx()
    {
        // === Base ===
        {
            std::vector<float> v = {
                // 4 vértices del tetraedro
                1,1,1, 0,0, 0,0,1,
                -1,-1,1, 1,0, 0,0,1,
                -1,1,-1, 1,1, 0,0,1,
                 1,-1,-1, 0,1, 0,0,1
            };
            std::vector<unsigned int> idx = {
                0,1,2,  0,3,1,  0,2,3,  1,3,2
            };
            PyrBase.Create(v, idx);
        }

        // === Stickers ===
        PyrStickers.clear();

        glm::vec3 A(1, 1, 1), B(-1, -1, 1), C(-1, 1, -1), D(1, -1, -1);
        glm::vec3 faces[4][3] = {
            {A,B,C}, {A,D,B}, {A,C,D}, {B,D,C}
        };

        int L = 3; // subdivisiones

        for (int f = 0; f < 4; f++)
        {
            glm::vec3 F0 = faces[f][0];
            glm::vec3 F1 = faces[f][1];
            glm::vec3 F2 = faces[f][2];

            glm::vec3 N = glm::normalize(glm::cross(F1 - F0, F2 - F0));

            for (int i = 0; i < L; i++)
            {
                for (int j = 0; j < L - i; j++)
                {
                    glm::vec3 p1 = bary(i, j, L, F0, F1, F2);
                    glm::vec3 p2 = bary(i + 1, j, L, F0, F1, F2);
                    glm::vec3 p3 = bary(i, j + 1, L, F0, F1, F2);

                    addSticker(p1, p2, p3, N);
                }
            }
        }
    }

    glm::vec3 bary(int i, int j, int L,
        const glm::vec3& A,
        const glm::vec3& B,
        const glm::vec3& C)
    {
        float u = float(i) / L;
        float v = float(j) / L;
        float w = 1.0f - u - v;
        return u * A + v * B + w * C;
    }

    void addSticker(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 N)
    {
        std::vector<float> v = {
            a.x,a.y,a.z, 0,0, N.x,N.y,N.z,
            b.x,b.y,b.z, 1,0, N.x,N.y,N.z,
            c.x,c.y,c.z, 0.5,1, N.x,N.y,N.z
        };
        std::vector<unsigned int> idx = { 0,1,2 };

        SimpleObjectGL obj;
        obj.Create(v, idx);
        PyrStickers.push_back(obj);
    }

    // ============================================================
    //  =====================  OCTAEDRO ===========================
    // ============================================================
    void CrearOctaedro()
    {
        std::vector<float> v = {
            // arriba
             0,1,0, 0.5,1, 0,1,0,
             1,0,0, 1,0.5, 1,0,0,
             0,0,1, 0.5,0.5, 0,0,1,

             0,1,0, 0.5,1, 0,1,0,
             0,0,1, 0.5,0.5, 0,0,1,
            -1,0,0, 0,0.5, -1,0,0,

            // abajo
             0,-1,0, 0.5,0, 0,-1,0,
             0,0,1, 0.5,0.5, 0,0,1,
             1,0,0, 1,0.5, 1,0,0,

             0,-1,0, 0.5,0, 0,-1,0,
            -1,0,0, 0,0.5, -1,0,0,
             0,0,1, 0.5,0.5, 0,0,1
        };
        std::vector<unsigned int> idx = {
            0,1,2, 3,4,5, 6,7,8, 9,10,11
        };
        Octaedro.Create(v, idx);
    }

    // ============================================================
    //  =================== DRAW METHODS ===========================
    // ============================================================
    void DrawCasa(const glm::mat4& M, GLint modelLoc)
    {
        CasaBody.Draw(modelLoc, M);
        CasaRoof.Draw(modelLoc, M);
        CasaPuerta.Draw(modelLoc, M);

        for (int i = 0; i < 6; i++)
            CasaVentanas[i].Draw(modelLoc, M);

        // Árbol izquierdo
        {
            glm::mat4 T = glm::translate(M, glm::vec3(-2.0f, -0.5f, 1.0f));
            ArbolTronco[0].Draw(modelLoc, T);
            ArbolCopa[0].Draw(modelLoc, T);
        }

        // Árbol derecho
        {
            glm::mat4 T = glm::translate(M, glm::vec3(2.0f, -0.5f, 1.0f));
            ArbolTronco[1].Draw(modelLoc, T);
            ArbolCopa[1].Draw(modelLoc, T);
        }
    }

    void DrawPyraminx(const glm::mat4& M, GLint modelLoc)
    {
        PyrBase.Draw(modelLoc, M);
        for (auto& s : PyrStickers)
            s.Draw(modelLoc, M);
    }

    void DrawOctaedro(const glm::mat4& M, GLint modelLoc)
    {
        Octaedro.Draw(modelLoc, M);
    }
};
