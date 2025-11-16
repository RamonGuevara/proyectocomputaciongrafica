#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <cmath>

// =====================================================================
//  Objeto simple (VAO, VBO, EBO)
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
            indexCount = (GLsizei)inds.size();
        }
        else
        {
            indexCount = (GLsizei)(verts.size() / 8);
        }

        // Posición
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
            8 * sizeof(GLfloat), (void*)0);
        glEnableVertexAttribArray(0);

        // Texcoords
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE,
            8 * sizeof(GLfloat),
            (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);

        // Normales
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
            8 * sizeof(GLfloat),
            (void*)(5 * sizeof(float)));
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
//  Clase con todos los objetos
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
    int PyrFaceStart[4] = { 0,0,0,0 };
    int PyrCountPerFace[4] = { 0,0,0,0 };

    SimpleObjectGL Octaedro;

public:
    void Init()
    {
        CrearCasa();
        CrearPyraminx();
        CrearOctaedro();
    }

private:
    // =================================================================
    //  Helpers de color plano
    // =================================================================
    void SetFlatColor(const glm::vec3& c) const
    {
        GLint prog = 0;
        glGetIntegerv(GL_CURRENT_PROGRAM, &prog);
        if (!prog) return;

        GLint useFlatLoc = glGetUniformLocation(prog, "useFlatColor");
        GLint flatColorLoc = glGetUniformLocation(prog, "flatColor");

        if (useFlatLoc >= 0) glUniform1i(useFlatLoc, GL_TRUE);
        if (flatColorLoc >= 0) glUniform3fv(flatColorLoc, 1, glm::value_ptr(c));
    }

    void DisableFlatColor() const
    {
        GLint prog = 0;
        glGetIntegerv(GL_CURRENT_PROGRAM, &prog);
        if (!prog) return;

        GLint useFlatLoc = glGetUniformLocation(prog, "useFlatColor");
        if (useFlatLoc >= 0) glUniform1i(useFlatLoc, GL_FALSE);
    }

    // =================================================================
    //  Geometría auxiliar: cilindro y cono (sin tapas)
    // =================================================================
    void CreateCylinder(SimpleObjectGL& obj,
        float radius, float yBottom, float yTop,
        int slices = 20)
    {
        std::vector<float> v;
        std::vector<unsigned int> idx;

        const float TWO_PI = 6.28318530718f;

        for (int i = 0; i <= slices; ++i)
        {
            float t = (float)i / (float)slices;
            float ang = t * TWO_PI;
            float cx = std::cos(ang);
            float cz = std::sin(ang);

            float x = radius * cx;
            float z = radius * cz;
            glm::vec3 n(cx, 0.0f, cz);

            // bottom
            v.insert(v.end(), {
                x, yBottom, z,
                t, 0.0f,
                n.x, n.y, n.z
                });
            // top
            v.insert(v.end(), {
                x, yTop, z,
                t, 1.0f,
                n.x, n.y, n.z
                });
        }

        for (int i = 0; i < slices; ++i)
        {
            unsigned int b0 = 2 * i;
            unsigned int t0 = b0 + 1;
            unsigned int b1 = 2 * (i + 1);
            unsigned int t1 = b1 + 1;

            idx.insert(idx.end(), { b0, b1, t1 });
            idx.insert(idx.end(), { b0, t1, t0 });
        }

        obj.Create(v, idx);
    }

    void CreateCone(SimpleObjectGL& obj,
        float radius, float yBase, float yTop,
        int slices = 20)
    {
        std::vector<float> v;
        std::vector<unsigned int> idx;

        const float TWO_PI = 6.28318530718f;
        glm::vec3 apex(0.0f, yTop, 0.0f);

        // punta
        v.insert(v.end(), {
            apex.x, apex.y, apex.z,
            0.5f, 1.0f,
            0.0f, 1.0f, 0.0f
            });
        unsigned int apexIndex = 0;

        for (int i = 0; i <= slices; ++i)
        {
            float t = (float)i / (float)slices;
            float ang = t * TWO_PI;
            float cx = std::cos(ang);
            float cz = std::sin(ang);

            glm::vec3 p(radius * cx, yBase, radius * cz);
            glm::vec3 edge = glm::normalize(
                glm::vec3(cx, (yTop - yBase) / radius, cz));

            v.insert(v.end(), {
                p.x, p.y, p.z,
                t, 0.0f,
                edge.x, edge.y, edge.z
                });
        }

        for (int i = 0; i < slices; ++i)
        {
            unsigned int i0 = apexIndex;
            unsigned int i1 = 1 + i;
            unsigned int i2 = 1 + i + 1;
            idx.insert(idx.end(), { i0, i1, i2 });
        }

        obj.Create(v, idx);
    }

    // =================================================================
    //  CASA
    // =================================================================
    void CrearCasa()
    {
        // ----- Cuerpo (cubo) -----
        {
            std::vector<float> v = {
                // pos               uv     normal
                -1.0f,-0.6f, 1.0f,  0,0,   0,0,1,
                 1.0f,-0.6f, 1.0f,  1,0,   0,0,1,
                 1.0f, 0.6f, 1.0f,  1,1,   0,0,1,
                -1.0f, 0.6f, 1.0f,  0,1,   0,0,1,

                -1.0f,-0.6f,-1.0f,  0,0,   0,0,-1,
                 1.0f,-0.6f,-1.0f,  1,0,   0,0,-1,
                 1.0f, 0.6f,-1.0f,  1,1,   0,0,-1,
                -1.0f, 0.6f,-1.0f,  0,1,   0,0,-1
            };

            std::vector<unsigned int> idx = {
                0,1,2, 2,3,0,      // frente
                1,5,6, 6,2,1,      // derecha
                5,4,7, 7,6,5,      // atrás
                4,0,3, 3,7,4,      // izquierda
                4,5,1, 1,0,4,      // piso
                3,2,6, 6,7,3       // techo plano
            };
            CasaBody.Create(v, idx);
        }

        // ----- Techo (pirámide cuadrangular) -----
        // base en y = 0.6, punta en y = 1.6
        {
            std::vector<float> v = {
                // base
                -1.0f, 0.6f,  1.0f,  0.0f, 0.0f,   0.0f,-1.0f, 0.0f,
                 1.0f, 0.6f,  1.0f,  1.0f, 0.0f,   0.0f,-1.0f, 0.0f,
                 1.0f, 0.6f, -1.0f,  1.0f, 1.0f,   0.0f,-1.0f, 0.0f,
                -1.0f, 0.6f, -1.0f,  0.0f, 1.0f,   0.0f,-1.0f, 0.0f,
                // punta
                 0.0f, 1.6f,  0.0f,  0.5f, 0.5f,   0.0f, 1.0f, 0.0f
            };

            std::vector<unsigned int> idx = {
                0,1,4,
                1,2,4,
                2,3,4,
                3,0,4
            };
            CasaRoof.Create(v, idx);
        }

        // ----- Puerta -----
        {
            std::vector<float> v = {
                -0.3f,-0.6f,1.01f, 0,0, 0,0,1,
                 0.3f,-0.6f,1.01f, 1,0, 0,0,1,
                 0.3f, 0.0f,1.01f, 1,1, 0,0,1,
                -0.3f, 0.0f,1.01f, 0,1, 0,0,1
            };
            std::vector<unsigned int> idx = { 0,1,2, 2,3,0 };
            CasaPuerta.Create(v, idx);
        }

        // ----- Ventanas (plantilla) -----
        for (int i = 0; i < 6; ++i)
        {
            std::vector<float> v = {
                -0.25f,0.1f,1.01f, 0,0, 0,0,1,
                 0.25f,0.1f,1.01f, 1,0, 0,0,1,
                 0.25f,0.4f,1.01f, 1,1, 0,0,1,
                -0.25f,0.4f,1.01f, 0,1, 0,0,1
            };
            std::vector<unsigned int> idx = { 0,1,2, 2,3,0 };
            CasaVentanas[i].Create(v, idx);
        }

        // ----- Árboles -----
        for (int i = 0; i < 2; ++i)
        {
            CreateCylinder(ArbolTronco[i],
                0.15f, -0.7f, 0.1f, 20);
            CreateCone(ArbolCopa[i],
                0.6f, 0.2f, 0.9f, 20);
        }
    }

    // =================================================================
    //  PYRAMINX
    // =================================================================
    void CrearPyraminx()
    {
        // base (tetraedro negro)
        {
            std::vector<float> v = {
                // pos        uv   normal aprox
                 1,  1,  1,   0,0, 0,0,1,
                -1, -1,  1,   1,0, 0,0,1,
                -1,  1, -1,   1,1, 0,0,1,
                 1, -1, -1,   0,1, 0,0,1
            };
            std::vector<unsigned int> idx = {
                0,1,2,
                0,3,1,
                0,2,3,
                1,3,2
            };
            PyrBase.Create(v, idx);
        }

        PyrStickers.clear();

        glm::vec3 A(1, 1, 1);
        glm::vec3 B(-1, -1, 1);
        glm::vec3 C(-1, 1, -1);
        glm::vec3 D(1, -1, -1);

        glm::vec3 faces[4][3] = {
            {A,B,C},
            {A,D,B},
            {A,C,D},
            {B,D,C}
        };

        const int L = 3; // subdivisiones

        for (int f = 0; f < 4; ++f)
        {
            glm::vec3 F0 = faces[f][0];
            glm::vec3 F1 = faces[f][1];
            glm::vec3 F2 = faces[f][2];

            glm::vec3 N = glm::normalize(glm::cross(F1 - F0, F2 - F0));

            int start = (int)PyrStickers.size();

            for (int i = 0; i < L; ++i)
            {
                for (int j = 0; j < L - i; ++j)
                {
                    glm::vec3 p1 = bary(i, j, L, F0, F1, F2);
                    glm::vec3 p2 = bary(i + 1, j, L, F0, F1, F2);
                    glm::vec3 p3 = bary(i, j + 1, L, F0, F1, F2);

                    // triángulo superior
                    addSticker(p1, p2, p3, N);

                    // triángulo complementario
                    if (i + j < L - 1)
                    {
                        glm::vec3 p4 = bary(i + 1, j + 1, L, F0, F1, F2);
                        addSticker(p2, p4, p3, N);
                    }
                }
            }

            int end = (int)PyrStickers.size();
            PyrFaceStart[f] = start;
            PyrCountPerFace[f] = end - start;
        }
    }

    glm::vec3 bary(int i, int j, int L,
        const glm::vec3& A,
        const glm::vec3& B,
        const glm::vec3& C)
    {
        float u = float(i) / float(L);
        float v = float(j) / float(L);
        float w = 1.0f - u - v;
        return u * A + v * B + w * C;
    }

    void addSticker(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 N)
    {
        // centro del triángulo
        glm::vec3 center = (a + b + c) / 3.0f;

        // 1) encoger el triángulo hacia el centro para aumentar la separación
        const float shrink = 0.90f;  // si quieres aún más separación, baja este valor
        a = center + (a - center) * shrink;
        b = center + (b - center) * shrink;
        c = center + (c - center) * shrink;

        // 2) pequeño offset hacia afuera para evitar z-fighting con la base
        const float eps = 0.01f;
        glm::vec3 offDir = glm::normalize(center);
        a += offDir * eps;
        b += offDir * eps;
        c += offDir * eps;

        std::vector<float> v = {
            a.x,a.y,a.z, 0.0f,0.0f, N.x,N.y,N.z,
            b.x,b.y,b.z, 1.0f,0.0f, N.x,N.y,N.z,
            c.x,c.y,c.z, 0.5f,1.0f, N.x,N.y,N.z
        };
        std::vector<unsigned int> idx = { 0,1,2 };

        SimpleObjectGL obj;
        obj.Create(v, idx);
        PyrStickers.push_back(obj);
    }

    // =================================================================
    //  OCTAEDRO con textura 1–8
    // =================================================================
    void CrearOctaedro()
    {
        // Vértices geométricos del octaedro
        glm::vec3 T(0, 1, 0);
        glm::vec3 B(0, -1, 0);
        glm::vec3 R(1, 0, 0);
        glm::vec3 L(-1, 0, 0);
        glm::vec3 F(0, 0, 1);
        glm::vec3 K(0, 0, -1);

        // Normales de cada cara (pre-normalizadas)
        glm::vec3 N0(-0.57735f, -0.57735f, -0.57735f); // T,R,F
        glm::vec3 N1(0.57735f, -0.57735f, -0.57735f); // T,F,L
        glm::vec3 N2(0.57735f, -0.57735f, 0.57735f); // T,L,K
        glm::vec3 N3(-0.57735f, -0.57735f, 0.57735f); // T,K,R
        glm::vec3 N4(-0.57735f, 0.57735f, -0.57735f); // B,F,R
        glm::vec3 N5(0.57735f, 0.57735f, -0.57735f); // B,L,F
        glm::vec3 N6(0.57735f, 0.57735f, 0.57735f); // B,K,L
        glm::vec3 N7(-0.57735f, 0.57735f, 0.57735f); // B,R,K

        std::vector<float> v;
        v.reserve(8 * 3 * 8); // 8 caras * 3 vértices * (pos+uv+normal)

        auto pushFace = [&](const glm::vec3& p0,
            const glm::vec3& p1,
            const glm::vec3& p2,
            const glm::vec3& n,
            int tileIndex)
            {
                // tileIndex 0..7 -> cuadros 1..8 de la textura (4 columnas x 2 filas)
                int col = tileIndex % 4;   // 0..3 (x)
                int row = tileIndex / 4;   // 0..1 (y)

                const float du = 1.0f / 4.0f;
                const float dv = 1.0f / 2.0f;

                // Región completa del cuadrito
                float u0 = col * du;
                float u1 = (col + 1) * du;
                float v0 = row * dv;
                float v1 = (row + 1) * dv;

                // ---- Escala para que el número se vea más pequeño ----
                float scale = 1.0f;   // 1.0 = tamaño completo; <1.0 = más pequeño

                // centro del tile
                float uCenter = 0.5f * (u0 + u1);
                float vCenter = 0.5f * (v0 + v1);

                // nuevo rectángulo reducido y centrado
                float halfU = (u1 - u0) * scale * 0.5f;
                float halfV = (v1 - v0) * scale * 0.5f;

                float u0s = uCenter - halfU;
                float u1s = uCenter + halfU;
                float v0s = vCenter - halfV;
                float v1s = vCenter + halfV;

                // coordenada U central (para el vértice superior del triángulo)
                float uMid = 0.5f * (u0s + u1s);

                float UV[3][2] = {
                     { uMid, v1s },  // vértice superior
                     { u1s,  v0s },  // base izquierda (antes u0s)
                     { u0s,  v0s }   // base derecha (antes u1s)
                };

                glm::vec3 P[3] = { p0, p1, p2 };

                for (int k = 0; k < 3; ++k)
                {
                    // posición
                    v.push_back(P[k].x);
                    v.push_back(P[k].y);
                    v.push_back(P[k].z);
                    // uv
                    v.push_back(UV[k][0]);
                    v.push_back(UV[k][1]);
                    // normal
                    v.push_back(n.x);
                    v.push_back(n.y);
                    v.push_back(n.z);
                }
            };

        // 8 caras -> tiles 0..7 (1..8)
        pushFace(T, R, F, N0, 0); // cara 1
        pushFace(T, F, L, N1, 1); // cara 2
        pushFace(T, L, K, N2, 2); // cara 3
        pushFace(T, K, R, N3, 3); // cara 4

        pushFace(B, F, R, N4, 4); // cara 5
        pushFace(B, L, F, N5, 5); // cara 6
        pushFace(B, K, L, N6, 6); // cara 7
        pushFace(B, R, K, N7, 7); // cara 8

        // índices: cada cara son 3 vértices únicos
        std::vector<unsigned int> idx;
        idx.reserve(8 * 3);
        for (unsigned int i = 0; i < 8 * 3; ++i)
            idx.push_back(i);

        Octaedro.Create(v, idx);
    }

public:
    // =================================================================
    //  DRAW
    // =================================================================
    void DrawCasa(const glm::mat4& M, GLint modelLoc)
    {
        const glm::vec3 ROJO(0.85f, 0.10f, 0.10f);
        const glm::vec3 AZUL(0.00f, 0.35f, 1.00f);
        const glm::vec3 VERDE(0.00f, 0.85f, 0.00f);
        const glm::vec3 VERDE_OSC(0.00f, 0.50f, 0.00f);
        const glm::vec3 CAFE(0.478f, 0.255f, 0.067f);

        SetFlatColor(ROJO);
        CasaBody.Draw(modelLoc, M);

        SetFlatColor(AZUL);
        CasaRoof.Draw(modelLoc, M);

        SetFlatColor(VERDE);
        CasaPuerta.Draw(modelLoc, M);

        SetFlatColor(VERDE);
        for (int i = 0; i < 6; ++i)
            CasaVentanas[i].Draw(modelLoc, M);

        // árbol izquierdo
        {
            glm::mat4 T = glm::translate(M, glm::vec3(-2.0f, -0.5f, 1.0f));
            SetFlatColor(CAFE);
            ArbolTronco[0].Draw(modelLoc, T);
            SetFlatColor(VERDE_OSC);
            ArbolCopa[0].Draw(modelLoc, T);
        }

        // árbol derecho
        {
            glm::mat4 T = glm::translate(M, glm::vec3(2.0f, -0.5f, 1.0f));
            SetFlatColor(CAFE);
            ArbolTronco[1].Draw(modelLoc, T);
            SetFlatColor(VERDE_OSC);
            ArbolCopa[1].Draw(modelLoc, T);
        }

        DisableFlatColor();
    }

    void DrawPyraminx(const glm::mat4& M, GLint modelLoc)
    {
        const glm::vec3 ROJO(0.85f, 0.10f, 0.10f);
        const glm::vec3 AZUL(0.00f, 0.35f, 1.00f);
        const glm::vec3 VERDE(0.00f, 0.85f, 0.00f);
        const glm::vec3 AMARILLO(0.95f, 0.90f, 0.08f);
        const glm::vec3 NEGRO(0.05f, 0.05f, 0.05f);

        SetFlatColor(NEGRO);
        PyrBase.Draw(modelLoc, M);

        GLboolean wasCull = glIsEnabled(GL_CULL_FACE);
        glDisable(GL_CULL_FACE);

        glm::vec3 faceColors[4] = { ROJO, VERDE, AZUL, AMARILLO };

        for (int f = 0; f < 4; ++f)
        {
            SetFlatColor(faceColors[f]);
            int start = PyrFaceStart[f];
            int count = PyrCountPerFace[f];
            for (int k = 0; k < count; ++k)
                PyrStickers[start + k].Draw(modelLoc, M);
        }

        if (wasCull) glEnable(GL_CULL_FACE);
        DisableFlatColor();
    }

    void DrawOctaedro(const glm::mat4& M, GLint modelLoc)
    {
        Octaedro.Draw(modelLoc, M);
    }
};
