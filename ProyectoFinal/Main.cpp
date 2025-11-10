#include <iostream>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "Camera.h"
#include "Model.h"
#include "Mesh.h"
#include "Texture.h"
#include "Window.h"

void CrearObjeto(GLuint& VAO, GLuint& VBO, GLuint& EBO,
    GLfloat* vertices, GLuint* indices, int tamV, int tamI);
void CrearObjetoSkyBox(GLuint& VAO, GLuint& VBO,
    GLfloat* vertices, int tamV);
void DibujarBarda(const glm::vec3& posicion, float rotacionY,
    float largo, GLuint VAO, GLuint textura, GLint modelLoc);
void ProcessInput(Window& window);
void Animation();

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

float deltaTime = 0.0f;
float lastFrame = 0.0f;

glm::vec3 pointLightPositions[] = {
    glm::vec3(0.0f, 2.0f, 0.0f)
};

bool puertaAbierta = false;
float rotPuerta = 0.0f;
float velocidadPuerta = 1.0f;

glm::vec3 ciervoPos(0.0f, 0.0f, 0.0f);
float ciervoRot = 0.0f;
float head = 0.0f;
float FLegs = 0.0f;
float RLegs = 0.0f;
bool ciervoAnim = false;
bool Ciervostep = false;

bool puertaTogglePressed = false;
bool ciervoTogglePressed = false;

glm::mat4 modelTemp(1.0f);

// Mundo: +Z frente, -Z atrás, +X derecha, -X izquierda, +Y arriba
const float WALL_HALF_LENGTH = 30.0f;
const float TOTAL_WALL_LENGTH = WALL_HALF_LENGTH * 2.0f;
const int   NUM_WALL_BLOCKS = 6;
const float BARDA_Y = -0.35f;
const float ENTRANCE_WIDTH = 6.0f;

// Puertas (espacio local)
const float DOOR_Z = 27.45f;
const glm::vec3 PUERTA_DER_PIVOT(1.433277f, 0.0f, DOOR_Z);
const glm::vec3 PUERTA_IZQ_PIVOT(-1.326733f, 0.0f, DOOR_Z);

// Centro local entre pivotes
const glm::vec3 GATE_LOCAL_REF = 0.5f * (PUERTA_DER_PIVOT + PUERTA_IZQ_PIVOT);

// Posición global del centro del conjunto arco+puertas
const glm::vec3 GATE_POS(0.0f, 0.0f, 30.0f);
const glm::vec3 GATE_SCALE(1.5f, 1.5f, 1.5f);

struct WallSegment {
    glm::vec3 pos;
    float rotY;
    float length;
};

int main()
{
    Window window(1280, 720, "Proyecto Final");
    if (window.GetGLFWwindow() == nullptr)
    {
        std::cerr << "Error al crear la ventana." << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    Shader lightingShader("Shader/Lighting.vert", "Shader/Lighting.frag");
    Shader skyboxShader("Shader/SkyBox.vert", "Shader/SkyBox.frag");

    lightingShader.Use();
    glUniform1i(glGetUniformLocation(lightingShader.Program, "texture_diffuse1"), 0);
    glUniform1i(glGetUniformLocation(lightingShader.Program, "texture_specular1"), 1);

    skyboxShader.Use();
    glUniform1i(glGetUniformLocation(skyboxShader.Program, "skybox"), 1);

    // Modelos entorno
    Model Piso((char*)"Piso.obj");
    Model Arco((char*)"ArcoTorii.obj");
    Model PuertaDer((char*)"PuertaDer.obj");
    Model PuertaIzq((char*)"PuertaIzq.obj");
    Model Rejas((char*)"Rejas.obj");

    // Ciervo adulto
    Model CiervoBody((char*)"Ciervo_cuerpo.obj");
    Model CiervoHead((char*)"Ciervo_head.obj");
    Model CiervoF_LeftLeg((char*)"Ciervo_fleftleg.obj");
    Model CiervoF_RightLeg((char*)"Ciervo_frightleg.obj");
    Model CiervoB_LeftLeg((char*)"Ciervo_bleftleg.obj");
    Model CiervoB_RightLeg((char*)"Ciervo_brightleg.obj");

    // Flora / entorno extra
    Model CiervoArbusto((char*)"Ciervo_arbusto.obj");
    Model CiervoRamas((char*)"Ciervo_arbustoramas.obj");
    Model ArbustosReja((char*)"Arbustos_reja.obj");
    Model Pinos((char*)"PinosHojas.obj");
    Model PinosRama((char*)"Models/PinosRama.obj");
    Model Aviario((char*)"Aviario.obj");
    Model AviarioV((char*)"VidrioAviario.obj");
    Model Flores((char*)"Flores.obj");
    Model Banca((char*)"BancasMadera.obj");

    // Ciervo bebé
    Model BabyDeerBody((char*)"CiervoBebe.obj");
    Model BabyDeerNeck((char*)"CiervoBebeCuello.obj");
    Model BabyDeerHead((char*)"CiervoBebeCabeza.obj");

    // Skybox
    GLfloat skyboxVertices[] = {
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };

    GLuint skyboxVAO, skyboxVBO;
    CrearObjetoSkyBox(skyboxVAO, skyboxVBO, skyboxVertices, sizeof(skyboxVertices));

    std::vector<const GLchar*> faces = {
        "SkyBox/right.jpg",
        "SkyBox/left.jpg",
        "SkyBox/top.jpg",
        "SkyBox/bottom.jpg",
        "SkyBox/back.jpg",
        "SkyBox/front.jpg"
    };

    GLuint cubemapTexture = TextureLoading::LoadCubemap(faces);

    // Barda (cubito base que luego escalamos para segmentos)
    GLfloat bardaVertices[] = {
        -0.5f, 0.0f,  0.5f,  0.0f, 0.0f,   0.0f, 0.0f, 1.0f,
         0.5f, 0.0f,  0.5f,  1.0f, 0.0f,   0.0f, 0.0f, 1.0f,
         0.5f, 1.0f,  0.5f,  1.0f, 1.0f,   0.0f, 0.0f, 1.0f,
        -0.5f, 1.0f,  0.5f,  0.0f, 1.0f,   0.0f, 0.0f, 1.0f,

        -0.5f, 0.0f, -0.5f,  1.0f, 0.0f,   0.0f, 0.0f,-1.0f,
         0.5f, 0.0f, -0.5f,  0.0f, 0.0f,   0.0f, 0.0f,-1.0f,
         0.5f, 1.0f, -0.5f,  0.0f, 1.0f,   0.0f, 0.0f,-1.0f,
        -0.5f, 1.0f, -0.5f,  1.0f, 1.0f,   0.0f, 0.0f,-1.0f,

        -0.5f, 0.0f, -0.5f,  0.0f, 0.0f,  -1.0f, 0.0f, 0.0f,
        -0.5f, 0.0f,  0.5f,  1.0f, 0.0f,  -1.0f, 0.0f, 0.0f,
        -0.5f, 1.0f,  0.5f,  1.0f, 1.0f,  -1.0f, 0.0f, 0.0f,
        -0.5f, 1.0f, -0.5f,  0.0f, 1.0f,  -1.0f, 0.0f, 0.0f,

         0.5f, 0.0f, -0.5f,  1.0f, 0.0f,   1.0f, 0.0f, 0.0f,
         0.5f, 0.0f,  0.5f,  0.0f, 0.0f,   1.0f, 0.0f, 0.0f,
         0.5f, 1.0f,  0.5f,  0.0f, 1.0f,   1.0f, 0.0f, 0.0f,
         0.5f, 1.0f, -0.5f,  1.0f, 1.0f,   1.0f, 0.0f, 0.0f,

        -0.5f, 1.0f,  0.5f,  0.0f, 0.0f,   0.0f, 1.0f, 0.0f,
         0.5f, 1.0f,  0.5f,  1.0f, 0.0f,   0.0f, 1.0f, 0.0f,
         0.5f, 1.0f, -0.5f,  1.0f, 1.0f,   0.0f, 1.0f, 0.0f,
        -0.5f, 1.0f, -0.5f,  0.0f, 1.0f,   0.0f, 1.0f, 0.0f,

        -0.5f, 0.0f,  0.5f,  0.0f, 0.0f,   0.0f,-1.0f, 0.0f,
         0.5f, 0.0f,  0.5f,  1.0f, 0.0f,   0.0f,-1.0f, 0.0f,
         0.5f, 0.0f, -0.5f,  1.0f, 1.0f,   0.0f,-1.0f, 0.0f,
        -0.5f, 0.0f, -0.5f,  0.0f, 1.0f,   0.0f,-1.0f, 0.0f
    };

    GLuint bardaIndices[] = {
        0, 1, 2,   0, 2, 3,
        5, 4, 7,   5, 7, 6,
        8, 9,10,   8,10,11,
        13,12,15,  13,15,14,
        16,17,18,  16,18,19,
        21,20,22,  22,20,23
    };

    GLuint bardaVAO, bardaVBO, bardaEBO;
    CrearObjeto(bardaVAO, bardaVBO, bardaEBO,
        bardaVertices, bardaIndices,
        sizeof(bardaVertices), sizeof(bardaIndices));

    GLuint bardaTextura = TextureLoading::LoadTexture("Textures/brick.png");

    glm::mat4 modelPiso = glm::scale(glm::mat4(1.0f), glm::vec3(1.05f));
    glm::mat4 modelRejas = glm::mat4(1.0f);

    // Transform base para arco + puertas
    glm::vec3 gateTranslate = GATE_POS - (GATE_LOCAL_REF * GATE_SCALE);
    glm::mat4 gateBase(1.0f);
    gateBase = glm::translate(gateBase, gateTranslate);
    gateBase = glm::scale(gateBase, GATE_SCALE);

    // Segmentos de barda
    const float segmentLength = TOTAL_WALL_LENGTH / (float)NUM_WALL_BLOCKS;
    const float halfEntrance = ENTRANCE_WIDTH * 0.5f;

    std::vector<WallSegment> wallSegments;
    wallSegments.reserve(NUM_WALL_BLOCKS * 4 + 4);

    // Trasera (-Z)
    {
        float startX = -WALL_HALF_LENGTH;
        for (int i = 0; i < NUM_WALL_BLOCKS; ++i)
        {
            float xL = startX + i * segmentLength;
            float xR = xL + segmentLength;
            float center = 0.5f * (xL + xR);
            float length = xR - xL;
            wallSegments.push_back({ glm::vec3(center, BARDA_Y, -WALL_HALF_LENGTH), 0.0f, length });
        }
    }

    // Frontal (+Z) con hueco
    {
        float startX = -WALL_HALF_LENGTH;
        float holeL = -halfEntrance;
        float holeR = halfEntrance;

        for (int i = 0; i < NUM_WALL_BLOCKS; ++i)
        {
            float xL = startX + i * segmentLength;
            float xR = xL + segmentLength;

            if (xR <= holeL || xL >= holeR)
            {
                float center = 0.5f * (xL + xR);
                float length = xR - xL;
                wallSegments.push_back({ glm::vec3(center, BARDA_Y, WALL_HALF_LENGTH), 0.0f, length });
            }
            else
            {
                if (xL < holeL)
                {
                    float leftLen = holeL - xL;
                    if (leftLen > 0.001f)
                    {
                        float leftCenter = xL + 0.5f * leftLen;
                        wallSegments.push_back({ glm::vec3(leftCenter, BARDA_Y, WALL_HALF_LENGTH), 0.0f, leftLen });
                    }
                }

                if (xR > holeR)
                {
                    float rightLen = xR - holeR;
                    if (rightLen > 0.001f)
                    {
                        float rightCenter = holeR + 0.5f * rightLen;
                        wallSegments.push_back({ glm::vec3(rightCenter, BARDA_Y, WALL_HALF_LENGTH), 0.0f, rightLen });
                    }
                }
            }
        }
    }

    // Izquierda (-X) y derecha (+X)
    {
        float startZ = -WALL_HALF_LENGTH;
        for (int i = 0; i < NUM_WALL_BLOCKS; ++i)
        {
            float zL = startZ + i * segmentLength;
            float zR = zL + segmentLength;
            float center = 0.5f * (zL + zR);
            float length = zR - zL;

            wallSegments.push_back({ glm::vec3(-WALL_HALF_LENGTH, BARDA_Y, center), 90.0f, length });
            wallSegments.push_back({ glm::vec3(WALL_HALF_LENGTH, BARDA_Y, center), 90.0f, length });
        }
    }

    // Cache uniforms
    lightingShader.Use();
    GLint modelLoc = glGetUniformLocation(lightingShader.Program, "model");
    GLint viewLoc = glGetUniformLocation(lightingShader.Program, "view");
    GLint projLoc = glGetUniformLocation(lightingShader.Program, "projection");
    GLint viewPosLoc = glGetUniformLocation(lightingShader.Program, "viewPos");

    GLint dirDirLoc = glGetUniformLocation(lightingShader.Program, "dirLight.direction");
    GLint dirAmbLoc = glGetUniformLocation(lightingShader.Program, "dirLight.ambient");
    GLint dirDifLoc = glGetUniformLocation(lightingShader.Program, "dirLight.diffuse");
    GLint dirSpecLoc = glGetUniformLocation(lightingShader.Program, "dirLight.specular");

    GLint pPosLoc = glGetUniformLocation(lightingShader.Program, "pointLights[0].position");
    GLint pAmbLoc = glGetUniformLocation(lightingShader.Program, "pointLights[0].ambient");
    GLint pDifLoc = glGetUniformLocation(lightingShader.Program, "pointLights[0].diffuse");
    GLint pSpecLoc = glGetUniformLocation(lightingShader.Program, "pointLights[0].specular");
    GLint pConstLoc = glGetUniformLocation(lightingShader.Program, "pointLights[0].constant");
    GLint pLinLoc = glGetUniformLocation(lightingShader.Program, "pointLights[0].linear");
    GLint pQuadLoc = glGetUniformLocation(lightingShader.Program, "pointLights[0].quadratic");

    GLint sPosLoc = glGetUniformLocation(lightingShader.Program, "spotLight.position");
    GLint sDirLoc = glGetUniformLocation(lightingShader.Program, "spotLight.direction");
    GLint sAmbLoc = glGetUniformLocation(lightingShader.Program, "spotLight.ambient");
    GLint sDifLoc = glGetUniformLocation(lightingShader.Program, "spotLight.diffuse");
    GLint sSpecLoc = glGetUniformLocation(lightingShader.Program, "spotLight.specular");
    GLint sConstLoc = glGetUniformLocation(lightingShader.Program, "spotLight.constant");
    GLint sLinLoc = glGetUniformLocation(lightingShader.Program, "spotLight.linear");
    GLint sQuadLoc = glGetUniformLocation(lightingShader.Program, "spotLight.quadratic");
    GLint sCutLoc = glGetUniformLocation(lightingShader.Program, "spotLight.cutOff");
    GLint sOuterCutLoc = glGetUniformLocation(lightingShader.Program, "spotLight.outerCutOff");

    GLint shininessLoc = glGetUniformLocation(lightingShader.Program, "material.shininess");
    GLint transpLoc = glGetUniformLocation(lightingShader.Program, "transparency");

    skyboxShader.Use();
    GLint skyViewLoc = glGetUniformLocation(skyboxShader.Program, "view");
    GLint skyProjLoc = glGetUniformLocation(skyboxShader.Program, "projection");

    // ---------- LOOP PRINCIPAL ----------
    while (!window.ShouldClose())
    {
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        window.PollEvents();
        ProcessInput(window);
        Animation();

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 projection = glm::perspective(
            glm::radians(camera.GetZoom()),
            (float)window.GetBufferWidth() / (float)window.GetBufferHeight(),
            0.1f, 100.0f
        );
        glm::mat4 view = camera.GetViewMatrix();

        // ----- SHADER ILUMINACIÓN -----
        lightingShader.Use();

        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
        glUniform3fv(viewPosLoc, 1, glm::value_ptr(camera.GetPosition()));

        // Luz direccional
        glUniform3f(dirDirLoc, -0.2f, -1.0f, -0.3f);
        glUniform3f(dirAmbLoc, 0.6f, 0.6f, 0.6f);
        glUniform3f(dirDifLoc, 0.6f, 0.6f, 0.6f);
        glUniform3f(dirSpecLoc, 0.3f, 0.3f, 0.3f);

        // Punto de luz
        glUniform3fv(pPosLoc, 1, glm::value_ptr(pointLightPositions[0]));
        glUniform3f(pAmbLoc, 0.8f, 0.8f, 0.8f);
        glUniform3f(pDifLoc, 0.8f, 0.8f, 0.8f);
        glUniform3f(pSpecLoc, 1.0f, 1.0f, 1.0f);
        glUniform1f(pConstLoc, 1.0f);
        glUniform1f(pLinLoc, 0.045f);
        glUniform1f(pQuadLoc, 0.075f);

        // Spotlight (cámara)
        glUniform3fv(sPosLoc, 1, glm::value_ptr(camera.GetPosition()));
        glUniform3fv(sDirLoc, 1, glm::value_ptr(camera.GetFront()));
        glUniform3f(sAmbLoc, 0.2f, 0.2f, 0.8f);
        glUniform3f(sDifLoc, 0.2f, 0.2f, 0.8f);
        glUniform3f(sSpecLoc, 0.0f, 0.0f, 0.0f);
        glUniform1f(sConstLoc, 1.0f);
        glUniform1f(sLinLoc, 0.3f);
        glUniform1f(sQuadLoc, 0.7f);
        glUniform1f(sCutLoc, glm::cos(glm::radians(12.0f)));
        glUniform1f(sOuterCutLoc, glm::cos(glm::radians(18.0f)));

        glUniform1f(shininessLoc, 5.0f);
        glUniform1i(transpLoc, 0);

        // ----- BARDA (segmentos) -----
        for (const auto& seg : wallSegments)
            DibujarBarda(seg.pos, seg.rotY, seg.length, bardaVAO, bardaTextura, modelLoc);

        // ----- PISO -----
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelPiso));
        Piso.Draw(lightingShader);

        // ----- ARCO -----
        {
            glm::mat4 modelArco = gateBase;
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelArco));
            Arco.Draw(lightingShader);
        }

        // ----- PUERTAS -----
        {
            // Puerta derecha
            glm::mat4 modelPuertaDer = gateBase;
            modelPuertaDer = glm::translate(modelPuertaDer, PUERTA_DER_PIVOT);
            modelPuertaDer = glm::rotate(modelPuertaDer, glm::radians(-rotPuerta), glm::vec3(0.0f, 1.0f, 0.0f));
            modelPuertaDer = glm::translate(modelPuertaDer, -PUERTA_DER_PIVOT);
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelPuertaDer));
            PuertaDer.Draw(lightingShader);

            // Puerta izquierda
            glm::mat4 modelPuertaIzq = gateBase;
            modelPuertaIzq = glm::translate(modelPuertaIzq, PUERTA_IZQ_PIVOT);
            modelPuertaIzq = glm::rotate(modelPuertaIzq, glm::radians(rotPuerta), glm::vec3(0.0f, 1.0f, 0.0f));
            modelPuertaIzq = glm::translate(modelPuertaIzq, -PUERTA_IZQ_PIVOT);
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelPuertaIzq));
            PuertaIzq.Draw(lightingShader);
        }

        // ----- CIERVO ADULTO -----
        {
            glm::mat4 modelCiervo(1.0f);
            modelCiervo = glm::translate(modelCiervo, ciervoPos);
            modelCiervo = glm::rotate(modelCiervo, glm::radians(ciervoRot), glm::vec3(0.0f, 1.0f, 0.0f));
            modelTemp = modelCiervo;

            // Cuerpo
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelCiervo));
            CiervoBody.Draw(lightingShader);

            // Cabeza
            glm::mat4 model = modelTemp;
            model = glm::translate(model, glm::vec3(-17.0f, 3.962f, 17.86f));
            model = glm::rotate(model, glm::radians(head), glm::vec3(0.0f, 0.0f, 1.0f));
            model = glm::translate(model, glm::vec3(17.0f, -3.962f, -17.86f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            CiervoHead.Draw(lightingShader);

            // Pata delantera izquierda
            model = modelTemp;
            model = glm::translate(model, glm::vec3(-18.1f, 2.005f, 16.41f));
            model = glm::rotate(model, glm::radians(FLegs), glm::vec3(-1.0f, 0.0f, 0.0f));
            model = glm::translate(model, glm::vec3(18.1f, -2.005f, -16.41f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            CiervoF_LeftLeg.Draw(lightingShader);

            // Pata delantera derecha
            model = modelTemp;
            model = glm::translate(model, glm::vec3(-17.12f, 2.339f, 16.19f));
            model = glm::rotate(model, glm::radians(FLegs), glm::vec3(1.0f, 0.0f, 0.0f));
            model = glm::translate(model, glm::vec3(17.12f, -2.339f, -16.19f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            CiervoF_RightLeg.Draw(lightingShader);

            // Pata trasera izquierda
            model = modelTemp;
            model = glm::translate(model, glm::vec3(-18.54f, 1.914f, 14.16f));
            model = glm::rotate(model, glm::radians(RLegs), glm::vec3(1.0f, 0.0f, 0.0f));
            model = glm::translate(model, glm::vec3(18.54f, -1.914f, -14.16f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            CiervoB_LeftLeg.Draw(lightingShader);

            // Pata trasera derecha
            model = modelTemp;
            model = glm::translate(model, glm::vec3(-17.04f, 2.047f, 13.88f));
            model = glm::rotate(model, glm::radians(RLegs), glm::vec3(-1.0f, 0.0f, 0.0f));
            model = glm::translate(model, glm::vec3(17.04f, -2.047f, -13.88f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            CiervoB_RightLeg.Draw(lightingShader);
        }

        // ----- REJAS -----
        {
            glm::mat4 model = modelRejas; // identidad
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            Rejas.Draw(lightingShader);
        }

        // ----- ARBUSTOS EN REJA -----
        {
            glm::mat4 model(1.0f);
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            ArbustosReja.Draw(lightingShader);
        }

        // ----- PINOS -----
        {
            glm::mat4 model(1.0f);
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            Pinos.Draw(lightingShader);

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            PinosRama.Draw(lightingShader);
        }

        // ----- CIERVO ARBUSTO / RAMAS -----
        {
            glm::mat4 model(1.0f);
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            CiervoArbusto.Draw(lightingShader);

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            CiervoRamas.Draw(lightingShader);
        }

        // ----- AVIARIO (opaco) -----
        {
            glm::mat4 model(1.0f);
            glUniform1i(transpLoc, 0);
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            Aviario.Draw(lightingShader);
        }

        // ----- AVIARIO VIDRIO (transparente) -----
        {
            glm::mat4 model(1.0f);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glUniform1i(transpLoc, 1);
            AviarioV.Draw(lightingShader);

            glDisable(GL_BLEND);
            glUniform1i(transpLoc, 0);
        }

        // ----- FLORES -----
        {
            glm::mat4 model(1.0f);
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            Flores.Draw(lightingShader);
        }

        // ----- BANCA -----
        {
            glm::mat4 model(1.0f);
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            Banca.Draw(lightingShader);
        }

        // ----- CIERVO BEBÉ (estático sencillo) -----
        {
            glm::mat4 model(1.0f);
            // aquí puedes hacer translate/scale si quieres moverlo
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glUniform1i(transpLoc, 0);

            BabyDeerBody.Draw(lightingShader);
            BabyDeerNeck.Draw(lightingShader);
            BabyDeerHead.Draw(lightingShader);
        }

        // ----- SKYBOX -----
        glDepthFunc(GL_LEQUAL);
        glDepthMask(GL_FALSE);

        skyboxShader.Use();
        glm::mat4 skyView = glm::mat4(glm::mat3(view));
        glUniformMatrix4fv(skyViewLoc, 1, GL_FALSE, glm::value_ptr(skyView));
        glUniformMatrix4fv(skyProjLoc, 1, GL_FALSE, glm::value_ptr(projection));

        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);

        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LESS);

        window.SwapBuffers();
    }

    return 0;
}

// ---------- RESTO DE FUNCIONES IGUAL QUE YA LAS TIENES ----------

void ProcessInput(Window& window)
{
    if (window.IsKeyPressed(GLFW_KEY_W) || window.IsKeyPressed(GLFW_KEY_UP))
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (window.IsKeyPressed(GLFW_KEY_S) || window.IsKeyPressed(GLFW_KEY_DOWN))
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (window.IsKeyPressed(GLFW_KEY_A) || window.IsKeyPressed(GLFW_KEY_LEFT))
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (window.IsKeyPressed(GLFW_KEY_D) || window.IsKeyPressed(GLFW_KEY_RIGHT))
        camera.ProcessKeyboard(RIGHT, deltaTime);

    if (window.IsKeyPressed(GLFW_KEY_Z))
        camera.ProcessKeyboard(UP, deltaTime);
    if (window.IsKeyPressed(GLFW_KEY_X))
        camera.ProcessKeyboard(DOWN, deltaTime);

    if (window.IsKeyPressed(GLFW_KEY_L)) pointLightPositions[0].x += 0.05f;
    if (window.IsKeyPressed(GLFW_KEY_J)) pointLightPositions[0].x -= 0.05f;
    if (window.IsKeyPressed(GLFW_KEY_I)) pointLightPositions[0].y += 0.05f;
    if (window.IsKeyPressed(GLFW_KEY_K)) pointLightPositions[0].y -= 0.05f;
    if (window.IsKeyPressed(GLFW_KEY_U)) pointLightPositions[0].z -= 0.05f;
    if (window.IsKeyPressed(GLFW_KEY_O)) pointLightPositions[0].z += 0.05f;

    float xOffset = window.GetXChange();
    float yOffset = window.GetYChange();
    if (xOffset != 0.0f || yOffset != 0.0f)
        camera.ProcessMouseMovement(xOffset, yOffset);

    bool pNow = window.IsKeyPressed(GLFW_KEY_P);
    if (pNow && !puertaTogglePressed)
    {
        puertaAbierta = !puertaAbierta;
        puertaTogglePressed = true;
    }
    else if (!pNow)
    {
        puertaTogglePressed = false;
    }

    bool bNow = window.IsKeyPressed(GLFW_KEY_B);
    if (bNow && !ciervoTogglePressed)
    {
        ciervoAnim = !ciervoAnim;
        ciervoTogglePressed = true;
    }
    else if (!bNow)
    {
        ciervoTogglePressed = false;
    }
}

void Animation()
{
    if (puertaAbierta)
    {
        if (rotPuerta < 90.0f) rotPuerta += velocidadPuerta;
    }
    else
    {
        if (rotPuerta > 0.0f) rotPuerta -= velocidadPuerta;
    }

    if (ciervoAnim)
    {
        if (!Ciervostep)
        {
            RLegs += 0.3f;
            FLegs += 0.3f;
            head += 0.15f;
            if (RLegs > 15.0f) Ciervostep = true;
        }
        else
        {
            RLegs -= 0.3f;
            FLegs -= 0.3f;
            head -= 0.15f;
            if (RLegs < -15.0f) Ciervostep = false;
        }
    }
    else
    {
        if (RLegs > 0.0f) RLegs -= 0.5f;
        if (RLegs < 0.0f) RLegs += 0.5f;
        if (FLegs > 0.0f) FLegs -= 0.5f;
        if (FLegs < 0.0f) FLegs += 0.5f;
        if (head > 0.0f) head -= 0.3f;
        if (head < 0.0f) head += 0.3f;
    }
}

void CrearObjeto(GLuint& VAO, GLuint& VBO, GLuint& EBO,
    GLfloat* vertices, GLuint* indices, int tamV, int tamI)
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, tamV, vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, tamI, indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
        8 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE,
        8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
        8 * sizeof(GLfloat), (GLvoid*)(5 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void CrearObjetoSkyBox(GLuint& VAO, GLuint& VBO,
    GLfloat* vertices, int tamV)
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, tamV, vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
        3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

void DibujarBarda(const glm::vec3& posicion, float rotacionY,
    float largo, GLuint VAO, GLuint textura, GLint modelLoc)
{
    glm::mat4 model(1.0f);
    model = glm::translate(model, posicion);
    model = glm::rotate(model, glm::radians(rotacionY), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(largo, 6.0f, 1.0f));

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textura);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
