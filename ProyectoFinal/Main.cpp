#include <iostream>
#include <vector>
#include <cmath>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>

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

//Animaciones ciervos
glm::vec3 ciervoPos(0.0f, 0.0f, 0.0f);
float ciervoRot = 0.0f;
float head = 0.0f;
float FLegs = 0.0f;
float RLegs = 0.0f;
bool ciervoAnim = false;
bool Ciervostep = false;
float ciervoSegLen = 5.0f;   // distancia por tramo
float ciervoStep = 0.01f;  // avance por frame 
float ciervoMoved = 0.0f;   // acumulado de la distancia recorrida en el tramo
int   ciervoDir = +1;     // +1 hacia +Z, -1 hacia -Z
bool  ciervoTurning = false;  // está girando
float ciervoTurnLeft = 180.0f; // grados por girar cuando toca
float ciervoTurnSpeed = 2.0f;   // grados por frame (giro)

bool puertaTogglePressed = false;
bool ciervoTogglePressed = false;

//Animaciones pajaros
glm::vec3 pajaro1pos(0.0f, 0.0f, 0.0f);
glm::vec3 tucan(0.0f, 0.0f, 0.0f);
float pajaro1Rot = 0.0f;
float ala1 = 0.0f;
float ala2 = 0.0f;
bool pajaro1Anim = false;
bool pajaro1step = false;
bool  pajaro1TogglePressed = false;   // para la tecla de toggle de todos los pajaros

glm::vec3 pajaro2pos(0.0f, 0.0f, 0.0f);
bool pajaro2Anim = false;
bool pajaro2step = false;
float pajaro2Rot = 0.0f;
float pajaro2head = 0.0f;
float pajaro2tail = 0.0f;


glm::vec3 pajaro3pos(0.0f, 0.0f, 0.0f);
bool pajaro3Anim = false;
bool pajaro3step = false;
float pajaro3Rot = 0.0f;
float pajaro3head = 0.0f;
float pajaro3tail = 0.0f;
float pico1 = 0.0f;
float pico2 = 0.0f;
const float picoMax = 12.0f;   // apertura máxima 
const float picoVel = 0.2f;    // velocidad de apertura

//  parámetros de vuelo
float flyAmpX = 0.6f;   // movimiento lateral (x)
float flyAmpY = 0.5f;   // subir/bajar (y)
float flyYawAmp = 8.0f;  // giro suave (yaw)
float flyHoverY = 0.6f;

// velocidades (grados/frame para alas; rad/frame para fase)
float wingAmp = 45.0f;  // amplitud de aleteo (±45°)
float wingHz = 0.45f;  // velocidad de aleteo
float flyHz = 0.05f;  // velocidad del ciclo de vuelo (trayectoria)

// fase interna
float pajaroPhase = 0.0f;

// Animacin oso polar
float pBearWalkTime = 0.0f;
float pBearFR_A = 0.0f;
float pBearFL_A = 0.0f;
float pBearBR_A = 0.0f;
float pBearBL_A = 0.0f;

// Animacin pingino
float pinguTime = 0.0f;
float pinguHead_A = 0.0f;
float pinguWings_A = 0.0f;

// Animacin foca
float sealTime = 0.0f;
float sealHead_A = 0.0f;  // rotacin cabeza arriba/abajo
float sealHands_A = 0.0f;  // rotacin aletas adelante/atrs


glm::mat4 modelTemp(1.0f);
glm::mat4 modelTemp2(1.0f);

// Mundo: +Z frente, -Z atrs, +X derecha, -X izquierda, +Y arriba
bool Ciervo2Anim = false;
bool ciervobebeTogglePressed = false;
float head2 = 0.0f;
float neck = 0.0f;

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

// Posicin global del centro del conjunto arco+puertas
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
    Model Cajas((char*)"cajas.obj");

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
    Model ArbolAv((char*)"aviarioarbol.obj");
    Model HojasAv((char*)"hojasaviario.obj");

    // Ciervo beb
    Model BabyDeerBody((char*)"CiervoBebe.obj");
    Model BabyDeerNeck((char*)"CiervoBebeCuello.obj");
    Model BabyDeerHead((char*)"CiervoBebeCabeza.obj");

    // Pinguino
    Model pinguH((char*)"pingu_head.obj");
    Model pinguB((char*)"pingu_body.obj");
    Model pinguW((char*)"pingu_wings.obj");

    // Piraa
    Model piranhaH((char*)"CabezaPirana.obj");
    Model piranhaT((char*)"ColaPirana.obj");

    // Foca
    Model sealB((char*)"seal_body.obj");
    Model sealH((char*)"seal_head.obj");
    Model sealHS((char*)"seal_hands.obj");
    Model sealT((char*)"seal_tail.obj");

    // Oso polar
    Model pBearBody((char*)"Polar_Bear_body.obj");
    Model pBearHead((char*)"Polar_Bear_head.obj");
    Model pBearFR((char*)"Polar_Bear_FR_Leg.obj");
    Model pBearFL((char*)"Polar_Bear_FL_Leg.obj");
    Model pBearBR((char*)"Polar_Bear_BR_Leg.obj");
    Model pBearBL((char*)"Polar_Bear_BL_Leg.obj");


    // Tiburn
    Model sharkB((char*)"Tiburon1_torso.obj");
    Model sharkH((char*)"Tiburon1.obj");
    Model sharkT((char*)"Tiburon1_cola.obj");

    // Iglu
    Model Iglu((char*)"Iglu.obj");

    //Pajaros
    Model Pajaro1Body((char*)"pajaro1body.obj");
    Model Pajaro1AlaDer((char*)"pajaro1der.obj");
    Model Pajaro1AlaIzq((char*)"pajaro1izq.obj");

    Model Pajaro2Body((char*)"pajaro2body.obj");
    Model Pajaro2Head((char*)"pajaro2head.obj");
    Model Pajaro2Tail((char*)"pajaro2tail.obj");

    Model TucanBody((char*)"tucanbody.obj");
    Model TucanHead((char*)"tucanhead.obj");
    Model TucanTail((char*)"tucantail.obj");
    Model TucanPico1((char*)"tucanpicoup.obj");
    Model TucanPico2((char*)"tucanpicodown.obj");

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

        // ----- SHADER ILUMINACIN -----
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

        // Spotlight (cmara)
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
            modelCiervo = glm::translate(modelCiervo, glm::vec3(-17.22f, 2.885f, 15.34f));
            modelCiervo = glm::translate(modelCiervo, ciervoPos);
            modelCiervo = glm::rotate(modelCiervo, glm::radians(ciervoRot), glm::vec3(0.0f, 1.0f, 0.0f));
            modelCiervo = glm::translate(modelCiervo, glm::vec3(17.22f, -2.885f, -15.34f));
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

        // ----- CIERVO BEBE -----
        {
            // matriz identidad
            glm::mat4 modelCiervoBebe(1.0f);
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelCiervoBebe));
            glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 0);
            // cuerpo (no se mueve ni rota)
            modelTemp = modelCiervoBebe;
            BabyDeerBody.Draw(lightingShader);

            //Neck
            glm::mat4 model1 = modelTemp;
            model1 = glm::translate(model1, glm::vec3(-15.555, 2.216f, 18.772f));
            model1 = glm::rotate(model1, glm::radians(neck), glm::vec3(1.0f, 0.0f, 0.0f));
            model1 = glm::translate(model1, glm::vec3(15.555, -2.216f, -18.772f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model1));
            BabyDeerNeck.Draw(lightingShader);

            //Head
            modelTemp2 = model1;
            model1 = modelTemp2;
            model1 = glm::translate(model1, glm::vec3(-15.32, 2.7725f, 19.114f));
            model1 = glm::rotate(model1, glm::radians(head2), glm::vec3(0.5f, 0.0f, 0.0f));
            model1 = glm::translate(model1, glm::vec3(15.32, -2.7725f, -19.114f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model1));
            BabyDeerHead.Draw(lightingShader);
        }

        // ----- PAJARO1 -----
        {
            glm::mat4 modelPajaro1(1.0f);
            modelPajaro1 = glm::translate(modelPajaro1, glm::vec3(17.06f, 8.834f, -10.47f));
            modelPajaro1 = glm::translate(modelPajaro1, pajaro1pos);
            modelPajaro1 = glm::rotate(modelPajaro1, glm::radians(pajaro1Rot), glm::vec3(0.0f, 1.0f, 0.0f));
            modelPajaro1 = glm::translate(modelPajaro1, glm::vec3(-17.06f, -8.834f, 10.47f));
            modelTemp = modelPajaro1;
            // Cuerpo
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelPajaro1));
            Pajaro1Body.Draw(lightingShader);

            glm::mat4 model = modelTemp;
            model = glm::translate(model, glm::vec3(16.38f, 9.123f, -10.18f));
            model = glm::rotate(model, glm::radians(ala1), glm::vec3(1.0f, 0.0f, 0.0f));
            model = glm::translate(model, glm::vec3(-16.38f, -9.123f, 10.18f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            Pajaro1AlaDer.Draw(lightingShader);

            model = modelTemp;
            model = glm::translate(model, glm::vec3(16.66f, 9.126f, -10.98f));
            model = glm::rotate(model, glm::radians(ala2), glm::vec3(1.0f, 0.0f, 0.0f));
            model = glm::translate(model, glm::vec3(-16.66f, -9.126f, 10.98f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            Pajaro1AlaIzq.Draw(lightingShader);
        }

        // ----- PAJARO2 -----
        {
            glm::mat4 modelPajaro2(1.0f);
            modelPajaro2 = glm::translate(modelPajaro2, glm::vec3(22.13f, 10.55f, -16.83f));
            modelPajaro2 = glm::translate(modelPajaro2, pajaro2pos);
            modelPajaro2 = glm::rotate(modelPajaro2, glm::radians(pajaro2Rot), glm::vec3(0.0f, 1.0f, 0.0f));
            modelPajaro2 = glm::translate(modelPajaro2, glm::vec3(-22.13f, -10.55f, 16.83f));
            modelTemp = modelPajaro2;
            // Cuerpo
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelPajaro2));
            Pajaro2Body.Draw(lightingShader);

            glm::mat4 model = modelTemp;
            model = glm::translate(model, glm::vec3(22.22f, 11.0f, -16.87f));
            model = glm::rotate(model, glm::radians(pajaro2head), glm::vec3(0.0f, 0.0f, 1.0f));
            model = glm::translate(model, glm::vec3(-22.22f, -11.0f, 16.87f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            Pajaro2Head.Draw(lightingShader);

            model = modelTemp;
            model = glm::translate(model, glm::vec3(21.11f, 9.278f, -17.48f));
            model = glm::rotate(model, glm::radians(pajaro2tail), glm::vec3(0.0f, 0.0f, 1.0f));
            model = glm::translate(model, glm::vec3(-21.11f, -9.278f, 17.48f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            Pajaro2Tail.Draw(lightingShader);
        }

        // ----- TUCAN -----
        {
            glm::mat4 modelPajaro3(1.0f);
            modelPajaro3 = glm::translate(modelPajaro3, glm::vec3(11.55f, 12.56f, -10.18f));
            modelPajaro3 = glm::translate(modelPajaro3, pajaro3pos);
            modelPajaro3 = glm::rotate(modelPajaro3, glm::radians(pajaro3Rot), glm::vec3(0.0f, 1.0f, 0.0f));
            modelPajaro3 = glm::translate(modelPajaro3, glm::vec3(-11.55f, -12.56f, 10.18f));
            // Cuerpo
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelPajaro3));
            modelTemp = modelPajaro3;
            TucanBody.Draw(lightingShader);

            glm::mat4 model1 = modelTemp;
            model1 = modelTemp;
            model1 = glm::translate(model1, glm::vec3(11.64f, 13.74f, -10.81f));
            model1 = glm::rotate(model1, glm::radians(pajaro3head), glm::vec3(0.0f, 0.0f, 1.0f));
            model1 = glm::translate(model1, glm::vec3(-11.64f, -13.74f, 10.81f));
            TucanHead.Draw(lightingShader);

            modelTemp2 = model1;
            model1 = modelTemp2;
            model1 = glm::translate(model1, glm::vec3(11.34, 13.64f, -10.06f));
            model1 = glm::rotate(model1, glm::radians(pico1), glm::vec3(-1.0f, 0.0f, 0.0f));
            model1 = glm::translate(model1, glm::vec3(-11.34, -13.64f, 10.06f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model1));
            TucanPico1.Draw(lightingShader);

            model1 = modelTemp2;
            model1 = glm::translate(model1, glm::vec3(11.34, 13.64f, -10.06f));
            model1 = glm::rotate(model1, glm::radians(pico2), glm::vec3(1.0f, 0.0f, 0.0f));
            model1 = glm::translate(model1, glm::vec3(-11.34, -13.64f, 10.06f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model1));
            TucanPico2.Draw(lightingShader);

            glm::mat4 model = modelTemp;
            model = glm::translate(model, glm::vec3(11.42f, 12.48f, -12.66f));
            model = glm::rotate(model, glm::radians(pajaro3tail), glm::vec3(1.0f, 0.0f, 0.0f));
            model = glm::translate(model, glm::vec3(-11.42f, -12.48f, 12.66f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            TucanTail.Draw(lightingShader);

        }

        // ----- REJAS -----
        {
            glm::mat4 model = modelRejas; // identidad
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            Rejas.Draw(lightingShader);
        }
        // ----- cajas -----
        {
            glm::mat4 model(1.0f);
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            Cajas.Draw(lightingShader);
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

        // ----- AVIARIO (opaco) -----
        {
            glm::mat4 model(1.0f);
            glUniform1i(transpLoc, 0);
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            Aviario.Draw(lightingShader);
        }

        // ----- AVIARIO VIDRIO (transparente) -----
        {
            glm::mat4 modelAv(1.0f);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelAv));
            glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 1);
            AviarioV.Draw(lightingShader);
            glDisable(GL_BLEND);
            glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 0);
        }
        // ----- ARBOL CENTRAL DEL AVIARIO -----
        {
            glm::mat4 model(1.0f);
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            ArbolAv.Draw(lightingShader);
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            HojasAv.Draw(lightingShader);
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

        // ----- PINGINO -----
        {
            glm::mat4 modelPingu(1.0f);
            modelPingu = glm::translate(modelPingu, glm::vec3(20.0f, 0.0f, 10.0f));
            modelPingu = glm::scale(modelPingu, glm::vec3(2.5f));
            modelPingu = glm::rotate(modelPingu, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

            // ----- Cuerpo -----
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelPingu));
            pinguB.Draw(lightingShader);

            // ----- Cabeza -----
            {
                glm::mat4 m = modelPingu;
                glm::vec3 HEAD_PIVOT(0.0f, 1.0f, 0.0f);
                m = glm::translate(m, HEAD_PIVOT);
                m = glm::rotate(m, glm::radians(pinguHead_A), glm::vec3(1.0f, 0.0f, 0.0f));
                m = glm::translate(m, -HEAD_PIVOT);
                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
                pinguH.Draw(lightingShader);
            }

            // ----- Alas (malla con ambas alas) -----
            {
                glm::mat4 m = modelPingu;
                const glm::vec3 WINGS_PIVOT(0.0f, 0.9f, 0.0f);

                m = glm::translate(m, WINGS_PIVOT);
                m = glm::rotate(m, glm::radians(pinguWings_A), glm::vec3(1.0f, 0.0f, 1.0f));
                m = glm::translate(m, -WINGS_PIVOT);

                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
                pinguW.Draw(lightingShader);
            }

        }
        // ----- PIRAÑA ------
        {
            //Head
            glm::mat4 modelPira(1.0f);
            modelPira = glm::translate(modelPira, glm::vec3(-1.0f, 0.0f, 0.0f));
            modelPira = glm::scale(modelPira, glm::vec3(0.3f, 0.3f, 0.3f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelPira));
            piranhaH.Draw(lightingShader);
            //Tail
            modelTemp = modelPira;
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelPira));
            piranhaT.Draw(lightingShader);
        }

        // ------- FOCA ------
        {
            glm::mat4 base(1.0f);

            // Posicin general de la foca (ajusta donde la quieras)
            base = glm::translate(base, glm::vec3(15.0f, 0.5f, 12.5f));
            base = glm::rotate(base, glm::radians(-90.0f), glm::vec3(0, 1, 0)); 
            base = glm::scale(base, glm::vec3(2.5f));                    

            // ----- Cuerpo -----
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(base));
            sealB.Draw(lightingShader);

            // ----- Cabeza (arriba/abajo) -----
            {
                glm::mat4 m = base;

                // Pivote aprox donde se une el cuello con el cuerpo 
                const glm::vec3 HEAD_PIVOT(0.0f, 0.35f, 0.7f);

                m = glm::translate(m, HEAD_PIVOT);
                m = glm::rotate(m, glm::radians(sealHead_A), glm::vec3(1.0f, 0.0f, 0.0f));
                m = glm::translate(m, -HEAD_PIVOT);

                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
                sealH.Draw(lightingShader);
            }

            // ----- Manos / aletas delanteras (adelante/atrs) -----
            {
                glm::mat4 m = base;
                const glm::vec3 HANDS_PIVOT(0.0f, 0.15f, 0.3f);

                m = glm::translate(m, HANDS_PIVOT);
                m = glm::rotate(m, glm::radians(sealHands_A), glm::vec3(1.0f, 0.0f, 0.0f));
                m = glm::translate(m, -HANDS_PIVOT);

                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
                sealHS.Draw(lightingShader);
            }
            {
                glm::mat4 m = base;

                // Pivote aprox en la base de la cola
                const glm::vec3 TAIL_PIVOT(0.0f, 0.1f, -0.6f);

                float tail_A = std::sin(sealTime * 1.5f) * 8.0f;

                m = glm::translate(m, TAIL_PIVOT);
                m = glm::rotate(m, glm::radians(tail_A), glm::vec3(0.0f, 1.0f, 0.0f));
                m = glm::translate(m, -TAIL_PIVOT);

                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
                sealT.Draw(lightingShader);
            }
        }

        // ------ OSO POLAR -------
        {
            glm::mat4 base(1.0f);

            // Posicin / escala / rotacin general del oso
            base = glm::translate(base, glm::vec3(20.0f, 0.0f, 15.0f));
            base = glm::scale(base, glm::vec3(2.5f));
            base = glm::rotate(base, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

            // ----- Cuerpo -----
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(base));
            pBearBody.Draw(lightingShader);

            // ----- Cabeza -----
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(base));
            pBearHead.Draw(lightingShader);

            // ----- Pivotes (en coordenadas LOCALES del oso, ajstalos a ojo) -----
            const glm::vec3 FR_PIVOT(0.3f, 0.3f, 0.4f); // Front Right
            const glm::vec3 FL_PIVOT(-0.3f, 0.3f, 0.4f); // Front Left
            const glm::vec3 BR_PIVOT(0.3f, 1.0f, -0.5f); // Back Right
            const glm::vec3 BL_PIVOT(-0.3f, 1.0f, -0.5f); // Back Left

            // Front Right Leg
            {
                glm::mat4 m = base;
                m = glm::translate(m, FR_PIVOT);
                m = glm::rotate(m, glm::radians(pBearFR_A), glm::vec3(1.0f, 0.0f, 0.0f));
                m = glm::translate(m, -FR_PIVOT);
                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
                pBearFR.Draw(lightingShader);
            }

            // Front Left Leg
            {
                glm::mat4 m = base;
                m = glm::translate(m, FL_PIVOT);
                m = glm::rotate(m, glm::radians(pBearFL_A), glm::vec3(1.0f, 0.0f, 0.0f));
                m = glm::translate(m, -FL_PIVOT);
                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
                pBearFL.Draw(lightingShader);
            }

            // Back Right Leg
            {
                glm::mat4 m = base;
                m = glm::translate(m, BR_PIVOT);
                m = glm::rotate(m, glm::radians(pBearBR_A), glm::vec3(1.0f, 0.0f, 0.0f));
                m = glm::translate(m, -BR_PIVOT);
                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
                pBearBR.Draw(lightingShader);
            }

            // Back Left Leg
            {
                glm::mat4 m = base;
                m = glm::translate(m, BL_PIVOT);
                m = glm::rotate(m, glm::radians(pBearBL_A), glm::vec3(1.0f, 0.0f, 0.0f));
                m = glm::translate(m, -BL_PIVOT);
                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
                pBearBL.Draw(lightingShader);
            }
        }

        // ------- TIBURON ----
        {
            //Body
            glm::mat4 modelTibu(1.0f);
            modelTibu = glm::translate(modelTibu, glm::vec3(0.0f, 0.0f, 1.0f));
            modelTibu = glm::scale(modelTibu, glm::vec3(0.01f, 0.01f, 0.01f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelTibu));
            sharkB.Draw(lightingShader);
            //Head
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelTibu));
            sharkH.Draw(lightingShader);
            //Tail
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelTibu));
            sharkT.Draw(lightingShader);
        }

        // ----- IGLU -----
        {
            glm::mat4 modelIglu(1.0f);

            // Posicin del igl (ajusta si quieres moverlo)
            modelIglu = glm::translate(modelIglu, glm::vec3(5.0f, 0.0f, 7.0f));

            // Escala del igl (sube/baja este valor segn el tamao del modelo)
            modelIglu = glm::scale(modelIglu, glm::vec3(0.8f));

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelIglu));
            glUniform1i(transpLoc, 0);      // Igl opaco
            Iglu.Draw(lightingShader);
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

    bool cNow = window.IsKeyPressed(GLFW_KEY_C);
    if (cNow && !pajaro1TogglePressed)
    {
        pajaro1Anim = !pajaro1Anim;
        pajaro2Anim = !pajaro2Anim;
        pajaro3Anim = !pajaro3Anim;
        pajaro1TogglePressed = true;
    }
    else if (!cNow)
    {
        pajaro1TogglePressed = false;
    }


    bool nNow = window.IsKeyPressed(GLFW_KEY_N);
    if (nNow && !ciervobebeTogglePressed) {
        Ciervo2Anim = !Ciervo2Anim;
        ciervobebeTogglePressed = true;
    }
    else if (!nNow) {
        ciervobebeTogglePressed = false;
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
        // 1) Animación de patas (lo tuyo de siempre)
        if (!Ciervostep) {        // State 1
            RLegs += 0.3f;
            FLegs += 0.3f;
            head += 0.3f;
            if (RLegs > 15.0f) Ciervostep = true;
        }
        else {                  // State 2
            RLegs -= 0.3f;
            FLegs -= 0.3f;
            head -= 0.3f;
            if (RLegs < -15.0f) Ciervostep = false;
        }

        // 2) Movimiento ping-pong
        if (!ciervoTurning) {
            // Caminar recto
            float dz = ciervoStep * ciervoDir;
            ciervoPos.z += dz;
            ciervoMoved += fabsf(dz);

            // ¿Llegó al final del tramo?
            if (ciervoMoved >= ciervoSegLen) {
                ciervoMoved = 0.0f;
                ciervoTurning = true;
                ciervoTurnLeft = 180.0f;
            }
        }
        else {
            // Giro en el sitio
            float dYaw = std::min(ciervoTurnSpeed, ciervoTurnLeft);
            ciervoRot += dYaw;
            ciervoTurnLeft -= dYaw;

            if (ciervoTurnLeft <= 0.0f) {
                ciervoTurning = false;
                ciervoDir *= -1;        // invertir dirección (+Z <-> -Z)
            }

            // Normalizar yaw para que no crezca sin límite
            if (ciervoRot >= 360.0f) ciervoRot -= 360.0f;
            if (ciervoRot < 0.0f) ciervoRot += 360.0f;
        }
    }
    // ----- OSO POLAR: caminar en su lugar -----
    pBearWalkTime += deltaTime * 4.0f;

    float swingA = std::sin(pBearWalkTime) * 20.0f;
    float swingB = std::sin(pBearWalkTime + glm::pi<float>()) * 20.0f;

    pBearFR_A = swingA;
    pBearBL_A = swingA;
    pBearFL_A = swingB;
    pBearBR_A = swingB;

    // ----- PINGINO: movimiento de cabeza y alas -----
    pinguTime += deltaTime * 3.0f;  // velocidad de animacin
    // alas se mueven en seno
    pinguWings_A = std::sin(pinguTime) * 35.0f; // +/-25 grados
    // cabeza en oposicin de fase (cuando alas van atrs, cabeza adelante)
    pinguHead_A = std::sin(pinguTime + glm::pi<float>()) * 15.0f; // +/-15 grados

    // ----- FOCA: arrastrndose -----
    sealTime += deltaTime * 2.5f; // velocidad de ciclo

    // Manos/aletitas: adelante y atrs (como paletas)
    // seno -> movimiento suave; rango aprox [-30, 30]
    sealHands_A = std::sin(sealTime) * 30.0f;

    // Cabeza: arriba/abajo en contrafase a las manos
    // cuando las manos empujan hacia atrs, la cabeza sube
    sealHead_A = std::sin(sealTime + glm::pi<float>()) * 15.0f;



    // Ciervo bebé
    if (Ciervo2Anim) {
        const float NECK_SPEED = 0.3f;  // cuello
        const float HEAD_SPEED = 0.3f;  // cabeza (ms lento)

        const float NECK_MAX = 16.0f;   // 
        const float HEAD_LIM = 15.0f;   //

        // estado persistente
        static int fase = 0;   // 0: bajar cuello, 1: cabecear, 2: subir cuello
        static int dir = -1;  // -1 baja cabeza, +1 sube
        static int reps = 0;   // 4 asentimientos

        if (fase == 0) { // bajar cuello hacia +20
            if (neck < NECK_MAX) neck += NECK_SPEED;
            else { neck = NECK_MAX; head2 = 0.0f; dir = -1; reps = 0; fase = 1; }
        }
        else if (fase == 1) { // cabecear 4 veces 15
            head2 += (dir * HEAD_SPEED);
            if (head2 <= -HEAD_LIM) { head2 = -HEAD_LIM; dir = +1; }
            if (head2 >= +HEAD_LIM) { head2 = +HEAD_LIM; dir = -1; reps++; }
            if (reps >= 2) fase = 2;
        }
        else { // fase == 2: subir cuello a 0 y centrar cabeza
            bool neckDone = false, headDone = false;

            if (neck > 0.0f) neck -= NECK_SPEED;
            else { neck = 0.0f; neckDone = true; }

            if (head2 > 0.0f) head2 -= HEAD_SPEED;
            else if (head2 < 0.0f) head2 += HEAD_SPEED;
            else headDone = true;

            if (std::fabs(head2) < 0.01f) { head2 = 0.0f; headDone = true; }

            // cuando ambos regresan a neutro, reinicia el ciclo
            if (neckDone && headDone) { fase = 0; dir = -1; reps = 0; }
        }
    }
    else {
        // anim apagada
        const float S1 = 0.6f, S2 = 0.8f;
        if (neck > 0.0f) neck = std::max(0.0f, neck - S1);
        if (head2 > 0.0f) head2 = std::max(0.0f, head2 - S2);
        if (head2 < 0.0f) head2 = std::min(0.0f, head2 + S2);
    }
   // ===== Pájaro 1: vuelo oscilatorio + aleteo =====
    if (pajaro1Anim) {
        pajaroPhase += flyHz;

        // Trayectoria: seno en X (lateral)
        pajaro1pos.x = flyAmpX * sinf(pajaroPhase);

        // ----- ALTURA 
        pajaro1pos.y = flyHoverY + flyAmpY * sinf(pajaroPhase * 1.3f);

        // Giro suave (yaw) acompasado al movimiento lateral
        pajaro1Rot = flyYawAmp * sinf(pajaroPhase);

        // Aleteo (alas en contrafase)
        float wingPhase = pajaroPhase * (wingHz / std::max(0.0001f, flyHz)); // desacopla frecuencia del vuelo
        ala1 = wingAmp * sinf(wingPhase);   // derecha
        ala2 = -wingAmp * sinf(wingPhase);   // izquierda
    }
    else {
        // Vuelve suave a neutro si se apaga la animación
        auto go0 = [](float& v, float s) { if (v > 0) v = std::max(0.0f, v - s); else v = std::min(0.0f, v + s); };
        go0(pajaro1pos.x, 0.05f);
        go0(pajaro1pos.y, 0.05f);   // vuelve hacia 0 cuando apagado (si prefieres volver a flyHoverY, ajusta aquí)
        go0(pajaro1Rot, 0.6f);
        go0(ala1, 2.0f);
        go0(ala2, 2.0f);
    }

    if (pajaro2Anim) {
        if (!pajaro2step) {        // State 1
            pajaro2head += 0.3f;
            pajaro2Rot += 0.5f;
            pajaro2tail += 1.0f;
            if (pajaro2Rot > 20.0f) pajaro2step = true;
        }
        else {                  // State 2
            pajaro2head -= 0.3f;
            pajaro2Rot -= 0.5f;
            pajaro2tail -= 1.0f;
            if (pajaro2Rot < -20.0f) pajaro2step = false;
        }
    }

    if (pajaro3Anim) {
        if (!pajaro3step) {              // State 1: abre
            pajaro3head += 0.3f;
            pajaro3Rot += 0.5f;
            pajaro3tail += 1.0f;

            pico1 += picoVel;
            pico2 += picoVel;

            // límite superior (no abre más que picoMax)
            if (pico1 > picoMax) pico1 = picoMax;
            if (pico2 > picoMax) pico2 = picoMax;

            if (pajaro3Rot > 20.0f) pajaro3step = true;
        }
        else {                         // State 2: cierra
            pajaro3head -= 0.3f;
            pajaro3Rot -= 0.5f;
            pajaro3tail -= 1.0f;

            pico1 -= picoVel;
            pico2 -= picoVel;

            // límite inferior (no se pasa de cerrado)
            if (pico1 < 0.0f) pico1 = 0.0f;
            if (pico2 < 0.0f) pico2 = 0.0f;

            if (pajaro3Rot < -20.0f) pajaro3step = false;
        }
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
