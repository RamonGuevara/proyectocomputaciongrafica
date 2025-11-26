//Importacion de las bibliotecas necesarias
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>

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
#include "Skybox.h"

// --- Cámara y personaje principal ---
bool thirdPerson = false;        // alternar vista (1ra / 3ra persona)
float cameraDistance = 8.5f;     // distancia en 3ra persona

glm::vec3 personajePos(0.0f, 0.0f, 0.0f);  // posición del personaje
float personajeRot = 0.0f;                 // rotación Y del personaje (en grados)

float moveSpeed = 4.0f;        // velocidad de desplazamiento
float turnSpeed = 90.0f;       // grados/seg para girar
bool followCharacter = true;   // si true, la cámara sigue al personaje

// --- Animación de caminata del personaje ---
bool  isWalking = false;  
float walkTime = 0.0f;    // fase del ciclo de caminata
float walkSpeed = 6.0f;    // velocidad del ciclo (rad/s aprox)

float maxArmSwingDeg = 35.0f;   // amplitud máxima de brazos (grados)
float maxLegSwingDeg = 25.0f;   // amplitud máxima de piernas (grados)

// ángulos actuales (en grados) que usarán los modelos
float armRightAngle = 0.0f;
float armLeftAngle = 0.0f;
float legRightAngle = 0.0f;
float legLeftAngle = 0.0f;

// -----------------------------------------------------------------------------
// declaracion de funciones auxiliares
// -----------------------------------------------------------------------------

/**
 * encargada de crear un objeto de dibujo generico, podriamos decir que es como un Abstract Abstract Factory
 * los realiza a partir de arreglos de vertices e index
 *
 * @param VAO  referencia al id del Vertex Array Object
 * @param VBO  referencia al id del Vertex Buffer Object
 * @param EBO  referencia al id del Element Buffer Object
 * @param vertices  array de vertices, pueden ser posiciones, normales, UV
 * @param indices   array index con glDrawElements.
 * @param tamV      no. total de floats en el arreglo de v
 * @param tamI      no. total de index
 */
    void CrearObjeto(GLuint& VAO, GLuint& VBO, GLuint& EBO,
        GLfloat* vertices, GLuint* indices, int tamV, int tamI);

/**
  * Crea el VAO y VBO para el skybox, usando solo los propios vertices, no se usan indices
  *
  * @param VAO      es la referencia al id del Vertex Array Object
  * @param VBO      referencia a id del Vertex Buffer Object
  * @param vertices array de vertices del cubo del skybox
  * @param tamV     numero total de floats en el arreglo de vértices
  */
void CrearObjetoSkyBox(GLuint& VAO, GLuint& VBO,
    GLfloat* vertices, int tamV);

/**
  * dibuja un segmento de barda escalando y rotando un cubo base
  *
  * @param posicion  centro del segmento dado en coordenadas de mundo
  * @param rotacionY rotación del eje Y en grados
  * @param largo     longitud del segmento, esto se usa para escalar el cubo
  * @param VAO       vao del cubo base a reutilizar
  * @param textura   id de la textura de barda
  * @param modelLoc  location del uniform 'model' en el shader
  */
void DibujarBarda(const glm::vec3& posicion, float rotacionY,
    float largo, GLuint VAO, GLuint textura, GLint modelLoc);

/**
  * procesa la entrada del usuario para controlar la camara, activar/desactivar animaciones o luces
  *
  * @param window referencia a la ventana principal
  */
void ProcessInput(Window& window);

/**
 * actualiza todas las animaciones de la escena en función de deltaTime y de los flags activos
 */
void Animation();

// -----------------------------------------------------------------------------
// camara, control de tiempo global, luces puntuales y luz ambiental
// -----------------------------------------------------------------------------

//camara principal que observa el zoologico
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

// tiempos para calcular deltaTime entre frames
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// posiciones de las luces dentro del zoologico
glm::vec3 pointLightPositions[] = {
    glm::vec3(25.0f, 7.0f, 0.0f),
    glm::vec3(-25.0f, 7.0f, 2.0f),
    glm::vec3(-1.0f, 7.0f, -23.0f),
    glm::vec3(-1.0f, 7.0f, 7.0f)
};

// luces puntuales on/off, el segundo parametro es para evitar rebotes al presionar la tecla
bool pointLightsOn = true;
bool pointLightsTogglePressed = false;

//luz ambiental
bool ambLightsOn = true;
bool ambLightsTogglePressed = false;

// -----------------------------------------------------------------------------
// puerta de entrada del zoológico
// -----------------------------------------------------------------------------
bool puertaAbierta = false;
float rotPuerta = 0.0f;
float velocidadPuerta = 1.0f;

// -----------------------------------------------------------------------------
// animacion para el habitat ciervos 2
// -----------------------------------------------------------------------------
bool animacionH2 = false;
bool animacionH2TogglePressed = false;

// animaciones del ciervo adulto: caminar, girar, cabeza
glm::vec3 ciervoPos(0.0f, 0.0f, 0.0f);
float ciervoRot = 0.0f;
float head = 0.0f;
float FLegs = 0.0f;
float RLegs = 0.0f;
bool Ciervostep = false;

// parametros de trayectoria del estilo ir y venir
float ciervoSegLen = 5.0f;   // distancia por tramo
float ciervoStep = 0.01f;  // avance por frame
float ciervoMoved = 0.0f;   // acumulado de la distancia recorrida en el tramo
int   ciervoDir = +1;     // +1 hacia +Z, -1 hacia -Z
bool  ciervoTurning = false;  // está girando
float ciervoTurnLeft = 180.0f; // grados por girar cuando toca
float ciervoTurnSpeed = 2.0f;   // grados por frame (giro)

//punto/rotacion originales del ciervo
glm::vec3 ciervoHomePos(0.0f, 0.0f, 0.0f);
float     ciervoHomeRot = 0.0f;

// velocidades para que el ciervo regrese a su posicion/rotacion original
float ciervoReturnPosStep = 0.02f;    // unidades por frame (XYZ)
float ciervoReturnRotStep = 2.0f;     // grados por frame

bool puertaTogglePressed = false;
bool ciervoTogglePressed = false; // Flag para el ciervo adulto

// -----------------------------------------------------------------------------
// Animación Ciervo Bebé
// -----------------------------------------------------------------------------
bool Ciervo2Anim = false;
bool ciervobebeTogglePressed = false;
float head2 = 0.0f;
float neck = 0.0f;

// -----------------------------------------------------------------------------
// animaciones de los pájaros: habitat de aves
// -----------------------------------------------------------------------------
glm::vec3 pajaro1pos(0.0f, 0.0f, 0.0f);
glm::vec3 tucan(0.0f, 0.0f, 0.0f);
//rotacion global, ala derecha e izquierda
float pajaro1Rot = 0.0f;
float ala1 = 0.0f;
float ala2 = 0.0f;
//animacion activa de los pajaros
bool pajaro1Anim = false;
bool pajaro1step = false;
bool  pajaro1TogglePressed = false;   // para la tecla de toggle de todos los pajaros

// pajaro 2
glm::vec3 pajaro2pos(0.0f, 0.0f, 0.0f);
bool pajaro2Anim = false;
bool pajaro2step = false;
float pajaro2Rot = 0.0f;
float pajaro2head = 0.0f;
float pajaro2tail = 0.0f;

//pajaro3
glm::vec3 pajaro3pos(0.0f, 0.0f, 0.0f);
bool pajaro3Anim = false;
bool pajaro3step = false;
float pajaro3Rot = 0.0f;
float pajaro3head = 0.0f;
float pajaro3tail = 0.0f;
// pico del ave, apertura y cierre
float pico1 = 0.0f;
float pico2 = 0.0f;
const float picoMax = 12.0f;   // apertura máxima
const float picoVel = 0.2f;    // velocidad de apertura

//  parámetros de vuelo pajaro 1
float flyAmpX = 0.6f;
float flyAmpY = 0.5f;
float flyYawAmp = 8.0f;
float flyHoverY = 0.6f;

// velocidades en grados/frame para alas, rad/frame para fase
float wingAmp = 45.0f;
float wingHz = 0.45f;
float flyHz = 0.05f;

// fase interna para animar al pajaro1 con funciones seno/coseno
float pajaroPhase = 0.0f;

// -----------------------------------------------------------------------------
// animacion para el habitat4: oso, pingüino, foca
// -----------------------------------------------------------------------------
bool animacionH4 = false;
bool animacionH4TogglePressed = false;
// --------------------------- animacion oso polar ---------------------------
// angulos de las patas: front right, front left, back right, back left
float pBearWalkTime = 0.0f;
float pBearFR_A = 0.0f;
float pBearFL_A = 0.0f;
float pBearBR_A = 0.0f;
float pBearBL_A = 0.0f;

glm::vec3 pBearPos(0.0f, 0.0f, 0.0f);
float pBearRot = 0.0f;
bool  pBearStepState = false;
// movimiento ping-pong a lo largo de un tramo recto
float pBearSegLen = 5.0f;
float pBearStep = 0.008f;
float pBearMoved = 0.0f;
int   pBearDir = -1;
bool  pBearTurning = false;
float pBearTurnLeft = 180.0f;
float pBearTurnSpeed = 2.0f;
// --------------------------- animacion pingüino ---------------------------
// al igual que otros aniamles mueve las alas y la cabeza
float pinguTime = 0.0f;
float pinguHead_A = 0.0f;
float pinguWings_A = 0.0f;

glm::vec3 pinguPos(0.0f, 0.0f, 0.0f);
float pinguRot = 0.0f;
bool  pinguStepState = false;
// movimiento ping-pong del pingüino
float pinguSegLen = 4.0f;
float pinguStep = 0.01f;
float pinguMoved = 0.0f;
int   pinguDir = -1;
bool  pinguTurning = false;
float pinguTurnLeft = 180.0f;
float pinguTurnSpeed = 2.0f;

// ----------------------------- animacion foca -----------------------------
float sealTime = 0.0f;
float sealHead_A = 0.0f;  // rotación cabeza arriba/abajo
float sealHands_A = 0.0f;  // rotación aletas adelante/atrás

glm::vec3 sealPos(0.0f, 0.0f, 0.0f);
float sealRot = 0.0f;
bool  sealStepState = false;
float sealSegLen = 6.0f;
float sealStep = 0.006f;
float sealMoved = 0.0f;
int   sealDir = -1;
bool  sealTurning = false;
float sealTurnLeft = 180.0f;
float sealTurnSpeed = 2.0f;

// -----------------------------------------------------------------------------
// animacion para el habitat1: tiburon y piraña
// -----------------------------------------------------------------------------
bool animacionH1 = false;
bool animacionH1TogglePressed = false;

// ----------------------------- animacion tiburon -----------------------------
float sharkTime = 0.0f;
float sharkHeadAngle = 0.0f;
float sharkTailA = 0.0f;
float sharkBodyAngle = 0.0f;  // angulo para oscilar el cuerpo

glm::vec3 sharkPos(0.0f);
float sharkRot = 0.0f;
float sharkZDir = 1.0f;   // dirección en Z: +1 adelante, -1 atrás
float sharkXDir = 1.0f;   // dirección en X: zigzag izquierda/derecha

// ----------------------------- animacion pirana -----------------------------
float piraHeadAngle = 0.0f;
float piraTailAngle = 0.0f;
float piraTime = 0.0f;  // tiempo para animación
glm::vec3 piranhaPos(0.0f);
float piranhaRot = 0.0f;
// direcciones ping-pong (+-1)
// ---esto se usa para rotacion suave sin trigonometria ---
float piraRotAngle = 0.0f;      // acumulador de rotación
float piraDirRot = 1.0f;      // +1 gira derecha, -1 gira izquierda





// -----------------------------------------------------------------------------
// matrices temporales para construir transformaciones, o sea model reutilizables
// -----------------------------------------------------------------------------
glm::mat4 modelTemp(1.0f);
glm::mat4 modelTemp2(1.0f);

// -----------------------------------------------------------------------------
// parametros del mundo y la barda exterior del zoo
// -----------------------------------------------------------------------------
// mundo: +Z frente, -Z atrás, +X derecha, -X izquierda, +Y arriba
const float WALL_HALF_LENGTH = 30.0f;
const float TOTAL_WALL_LENGTH = WALL_HALF_LENGTH * 2.0f;
const int   NUM_WALL_BLOCKS = 6;
const float BARDA_Y = -0.35f;
const float ENTRANCE_WIDTH = 6.0f;


// -----------------------------------------------------------------------------
// Parámetros de las puertas de entrada (en espacio local del modelo)
// -----------------------------------------------------------------------------
// DOOR_Z es la coordenada Z donde se ubican las puertas respecto al origen del arco
// los pivotes representan el punto de giro de cada hoja
const float DOOR_Z = 27.45f;
const glm::vec3 PUERTA_DER_PIVOT(1.433277f, 0.0f, DOOR_Z);
const glm::vec3 PUERTA_IZQ_PIVOT(-1.326733f, 0.0f, DOOR_Z);

// centro local entre pivotes
const glm::vec3 GATE_LOCAL_REF = 0.5f * (PUERTA_DER_PIVOT + PUERTA_IZQ_PIVOT);

// posicion global del centro del conjunto arco+puertas
const glm::vec3 GATE_POS(0.0f, 0.0f, 30.0f);
const glm::vec3 GATE_SCALE(1.5f, 1.5f, 1.5f);

// -----------------------------------------------------------------------------
// estructura para describir un segmento de barda
// -----------------------------------------------------------------------------
// Cada WallSegment almacena la posición del centro del tramo, su rotación sobre Y, y el largo
struct WallSegment {
    glm::vec3 pos;
    float rotY;
    float length;
};

int main()
{
    // -------------------------------------------------------------------------
    // creacion de la ventana y config OpenGL
    // -------------------------------------------------------------------------
    Window window(1920, 1080, "Proyecto Final");
    if (window.GetGLFWwindow() == nullptr)
    {
        std::cerr << "Error al crear la ventana." << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    // -------------------------------------------------------------------------
    // carga de shaders principales: i luminacion y el skybox
    // -------------------------------------------------------------------------
    Shader lightingShader("Shader/Lighting.vert", "Shader/Lighting.frag");
    Shader skyboxShader("Shader/SkyBox.vert", "Shader/SkyBox.frag");
    lightingShader.Use();
    glUniform1i(glGetUniformLocation(lightingShader.Program, "texture_diffuse1"), 0);
    glUniform1i(glGetUniformLocation(lightingShader.Program, "texture_specular1"), 1);

    // -------------------------------------------------------------------------
    // carga de modelos del entorno general
    // -------------------------------------------------------------------------
    Model Piso((char*)"Piso.obj");
    Model Arco((char*)"ArcoTorii.obj");
    Model PuertaDer((char*)"PuertaDer.obj");
    Model PuertaIzq((char*)"PuertaIzq.obj");
    Model Rejas((char*)"Rejas.obj");
    Model Cajas((char*)"cajas.obj");

    // -------------------------------------------------------------------------
    // modelos del hábitat 2 - Ciervo adulto, aqui se declaran cuerpo y partes articuladas: cabeza y patas
    // -------------------------------------------------------------------------
    Model CiervoBody((char*)"Ciervo_cuerpo.obj");
    Model CiervoHead((char*)"Ciervo_head.obj");
    Model CiervoF_LeftLeg((char*)"Ciervo_fleftleg.obj");
    Model CiervoF_RightLeg((char*)"Ciervo_frightleg.obj");
    Model CiervoB_LeftLeg((char*)"Ciervo_bleftleg.obj");
    Model CiervoB_RightLeg((char*)"Ciervo_brightleg.obj");

    // -------------------------------------------------------------------------
    // flora y entorno extra: arbustos, pinos, aviario, mobiliario
    // -------------------------------------------------------------------------
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

    // -------------------------------------------------------------------------
    // modelos del habitat 2 - Ciervo bebe
    // -------------------------------------------------------------------------
    Model BabyDeerBody((char*)"CiervoBebe.obj");
    Model BabyDeerNeck((char*)"CiervoBebeCuello.obj");
    Model BabyDeerHead((char*)"CiervoBebeCabeza.obj");

    // -------------------------------------------------------------------------
    // modelos del habitat 4 - Pingüino
    // -------------------------------------------------------------------------
    Model pinguH((char*)"pingu_head.obj");
    Model pinguB((char*)"pingu_body.obj");
    Model pinguW((char*)"pingu_wings.obj");

    // -------------------------------------------------------------------------
    // modelos del habitat 1 - Piraña
    // -------------------------------------------------------------------------
    Model piranhaH((char*)"CabezaPirana.obj");
    Model piranhaT((char*)"ColaPirana.obj");

    // -------------------------------------------------------------------------
    // modelos del habitat 4 - Foca
    // -------------------------------------------------------------------------
    Model sealB((char*)"seal_body.obj");
    Model sealH((char*)"seal_head.obj");
    Model sealHS((char*)"seal_hands.obj");
    Model sealT((char*)"seal_tail.obj");

    // -------------------------------------------------------------------------
    // modelos del habitat 4 - Oso polar
    // -------------------------------------------------------------------------
    Model pBearBody((char*)"Polar_Bear_body.obj");
    Model pBearHead((char*)"Polar_Bear_head.obj");
    Model pBearFR((char*)"Polar_Bear_FR_Leg.obj");
    Model pBearFL((char*)"Polar_Bear_FL_Leg.obj");
    Model pBearBR((char*)"Polar_Bear_BR_Leg.obj");
    Model pBearBL((char*)"Polar_Bear_BL_Leg.obj");

    // -------------------------------------------------------------------------
    // modelos del habitat 1 - Tiburón
    // -------------------------------------------------------------------------
    Model sharkB((char*)"Tiburon1_torso.obj");
    Model sharkH((char*)"Tiburon1.obj");
    Model sharkT((char*)"Tiburon1_cola.obj");

    // -------------------------------------------------------------------------
    // modelos de entorno artico/acuario: iglu, bardas, pecera, corales
    // -------------------------------------------------------------------------
    // Iglu
    Model Iglu((char*)"Iglu.obj");

    // Modelos de entorno Hábitat 4 (Ártico/Acuático)
    Model BardaMetal((char*)"bardametal1.obj");
    Model BardaVidrio((char*)"bardavidrio.obj");
    Model Pecera((char*)"pecera.obj");
    Model Agua((char*)"aguafinal.obj");
    Model Arena((char*)"arena.obj");
    Model Coral1((char*)"coral1.obj");
    Model Coral2((char*)"coral2.obj");
    Model Coral3((char*)"coral3.obj");
    Model Banquitas((char*)"banquitas.obj");

    // -------------------------------------------------------------------------
    // modelos del habitat de aves pajaro y tucan
    // -------------------------------------------------------------------------
    // Pájaros
    Model Pajaro1Body((char*)"pajaro1body.obj");
    Model Pajaro1AlaDer((char*)"pajaro1der.obj");
    Model Pajaro1AlaIzq((char*)"pajaro1izq.obj");

    //tucan
    Model Pajaro2Body((char*)"pajaro2body.obj");
    Model Pajaro2Head((char*)"pajaro2head.obj");
    Model Pajaro2Tail((char*)"pajaro2tail.obj");

    Model TucanBody((char*)"tucanbody.obj");
    Model TucanHead((char*)"tucanhead.obj");
    Model TucanTail((char*)"tucantail.obj");
    Model TucanPico1((char*)"tucanpicoup.obj");
    Model TucanPico2((char*)"tucanpicodown.obj");

    // ---------------------------------------------------------
    // MODELO DEL PERSONAJE PRINCIPAL (multipartes)
    // ---------------------------------------------------------
    Model P_Cabeza((char*)"Cabeza.obj");
    Model P_Torso((char*)"Torso.obj");
    Model P_BrazoDer((char*)"Brazo_derecho.obj");
    Model P_BrazoIzq((char*)"Brazo_izquierdo.obj");
    Model P_PiernaDer((char*)"Pierna_derecha.obj");
    Model P_PiernaIzq((char*)"Pierna_izquierda.obj");

    // Rutas del cubemap de día
    std::vector<const GLchar*> facesDay = {
        "SkyBox/rightDay.png",
        "SkyBox/leftDay.png",
        "SkyBox/topDay.png",
        "SkyBox/bottomDay.png",
        "SkyBox/backDay.png",
        "SkyBox/frontDay.png"
    };

    // Rutas del cubemap de noche
    std::vector<const GLchar*> facesNight = {
        "SkyBox/rightNight.png",
        "SkyBox/leftNight.png",
        "SkyBox/topNight.png",
        "SkyBox/bottomNight.png",
        "SkyBox/backNight.png",
        "SkyBox/frontNight.png"
    };

    // Objeto skybox (usa el shader que ya tenías)
    Skybox skybox(skyboxShader,facesDay, facesNight, 5.0f);


    // -----------------------------------------------------------------------------
    // geometría y recursos de la barda perimetral
    // -----------------------------------------------------------------------------
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

    // index para dibujar las 6 caras, que son 2 triangulos por cara
    GLuint bardaIndices[] = {
        0, 1, 2,   0, 2, 3,
        5, 4, 7,   5, 7, 6,
        8, 9,10,   8,10,11,
        13,12,15,  13,15,14,
        16,17,18,  16,18,19,
        21,20,22,  22,20,23
    };
    // creamos el VAO/VBO/EBO de la barda para reutilizarlo en todos los segmentos
    GLuint bardaVAO, bardaVBO, bardaEBO;
    CrearObjeto(bardaVAO, bardaVBO, bardaEBO,
        bardaVertices, bardaIndices,
        sizeof(bardaVertices), sizeof(bardaIndices));

    // texturas usadas en distintos objetos “simples” basados en el cubo/barda
    GLuint bardaTextura = TextureLoading::LoadTexture("Textures/brick.png");
    GLuint cubeTexture = TextureLoading::LoadTexture("Textures/plain.png");
    GLuint lampTexture = TextureLoading::LoadTexture("Textures/lamparas.jpg");
    GLuint maderaTextura = TextureLoading::LoadTexture("Textures/maderacaja.png");
    GLuint cartelTextura = TextureLoading::LoadTexture("Textures/venadocartel.jpg");
    GLuint cartelTextura1 = TextureLoading::LoadTexture("Textures/carteloso.jpg");
    GLuint cartelTextura2 = TextureLoading::LoadTexture("Textures/cartelacuario.jpg");


    // -----------------------------------------------------------------------------
    // geometría generica de cubo
    // -----------------------------------------------------------------------------
    // cubeVertices define un cubo unitario centrado en el origen, con posición,
    // coordenadas de textura y normales por vértice. Se reutiliza con distintas
    // transformaciones y texturas para varios props de la escena.
    GLfloat cubeVertices[] = {

        // Frente (+Z)
        -0.5f, -0.5f,  0.5f,   0.0f, 0.0f,    0.0f, 0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,   1.0f, 0.0f,    0.0f, 0.0f, 1.0f,
         0.5f,  0.5f,  0.5f,   1.0f, 1.0f,    0.0f, 0.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,   0.0f, 1.0f,    0.0f, 0.0f, 1.0f,

        // Detrás (-Z)
        -0.5f, -0.5f, -0.5f,   1.0f, 0.0f,    0.0f, 0.0f,-1.0f,
         0.5f, -0.5f, -0.5f,   0.0f, 0.0f,    0.0f, 0.0f,-1.0f,
         0.5f,  0.5f, -0.5f,   0.0f, 1.0f,    0.0f, 0.0f,-1.0f,
        -0.5f,  0.5f, -0.5f,   1.0f, 1.0f,    0.0f, 0.0f,-1.0f,

        // Izquierda (-X)
        -0.5f, -0.5f, -0.5f,   0.0f, 0.0f,   -1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,   1.0f, 0.0f,   -1.0f, 0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,   1.0f, 1.0f,   -1.0f, 0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,   0.0f, 1.0f,   -1.0f, 0.0f, 0.0f,

        // Derecha (+X)
         0.5f, -0.5f, -0.5f,   1.0f, 0.0f,    1.0f, 0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,   0.0f, 0.0f,    1.0f, 0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,   0.0f, 1.0f,    1.0f, 0.0f, 0.0f,
         0.5f,  0.5f, -0.5f,   1.0f, 1.0f,    1.0f, 0.0f, 0.0f,

         // Abajo (-Y)
         -0.5f, -0.5f, -0.5f,   0.0f, 1.0f,    0.0f,-1.0f, 0.0f,
          0.5f, -0.5f, -0.5f,   1.0f, 1.0f,    0.0f,-1.0f, 0.0f,
          0.5f, -0.5f,  0.5f,   1.0f, 0.0f,    0.0f,-1.0f, 0.0f,
         -0.5f, -0.5f,  0.5f,   0.0f, 0.0f,    0.0f,-1.0f, 0.0f,

         // Arriba (+Y)
         -0.5f,  0.5f, -0.5f,   0.0f, 0.0f,    0.0f, 1.0f, 0.0f,
          0.5f,  0.5f, -0.5f,   1.0f, 0.0f,    0.0f, 1.0f, 0.0f,
          0.5f,  0.5f,  0.5f,   1.0f, 1.0f,    0.0f, 1.0f, 0.0f,
         -0.5f,  0.5f,  0.5f,   0.0f, 1.0f,    0.0f, 1.0f, 0.0f
    };

    // otra vez indices para dibujar un cubo
    GLuint cubeIndices[] = {
        // Frente
        0, 1, 2,  2, 3, 0,
        // Detrás
        4, 5, 6,  6, 7, 4,
        // Izquierda
        8, 9,10, 10,11, 8,
        // Derecha
        12,13,14, 14,15,12,
        // Abajo
        16,17,18, 18,19,16,
        // Arriba
        20,21,22, 22,23,20
    };





    // creamos el objeto de dibujo para el cubo unitario. Este se reutiliza para
    // cajas, lámparas, carteles, etc., cambiando únicamente textura y transformaciones
    GLuint cubeVAO, cubeVBO, cubeEBO;
    CrearObjeto(cubeVAO, cubeVBO, cubeEBO,
        cubeVertices, cubeIndices,
        sizeof(cubeVertices), sizeof(cubeIndices));

    // -----------------------------------------------------------------------------
    // transformacion base para el conjunto arco + puertas
    // -----------------------------------------------------------------------------
    glm::mat4 modelPiso = glm::scale(glm::mat4(1.0f), glm::vec3(1.05f));
    glm::mat4 modelRejas = glm::mat4(1.0f);

    // gateTranslate posiciona el conjunto arco+puertas en el mundo, compensando que
    // los pivotes de las puertas están en coordenadas locales y además se escala todo
    glm::vec3 gateTranslate = GATE_POS - (GATE_LOCAL_REF * GATE_SCALE);
    glm::mat4 gateBase(1.0f);
    gateBase = glm::translate(gateBase, gateTranslate);
    gateBase = glm::scale(gateBase, GATE_SCALE);

    // -----------------------------------------------------------------------------
    // construccion de los segmentos de barda: perimetro del zoo
    // -----------------------------------------------------------------------------
    // cada lado de la barda se divide en 'NUM_WALL_BLOCKS' segmentos de longitud igual
    const float segmentLength = TOTAL_WALL_LENGTH / (float)NUM_WALL_BLOCKS;
    const float halfEntrance = ENTRANCE_WIDTH * 0.5f;

    std::vector<WallSegment> wallSegments;
    wallSegments.reserve(NUM_WALL_BLOCKS * 4 + 4);

    // --------------------------- barda trasera (-Z) ------------------------------
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

    // --------------------------- barda frontal (+Z) con hueco --------------------
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

    // --------------------------- Bardas laterales (-X y +X) ----------------------
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

    // -----------------------------------------------------------------------------
    // cache de locations de uniforms del shader de iluminacion
    // -----------------------------------------------------------------------------
    // para evitar llamar glGetUniformLocation dentro del loop, se guardan los índices
    // de los uniforms importantes model, view, projection, luces, material
    lightingShader.Use();
    GLint modelLoc = glGetUniformLocation(lightingShader.Program, "model");
    GLint viewLoc = glGetUniformLocation(lightingShader.Program, "view");
    GLint projLoc = glGetUniformLocation(lightingShader.Program, "projection");
    GLint viewPosLoc = glGetUniformLocation(lightingShader.Program, "viewPos");

    GLint dirDirLoc = glGetUniformLocation(lightingShader.Program, "dirLight.direction");
    GLint dirAmbLoc = glGetUniformLocation(lightingShader.Program, "dirLight.ambient");
    GLint dirDifLoc = glGetUniformLocation(lightingShader.Program, "dirLight.diffuse");
    GLint dirSpecLoc = glGetUniformLocation(lightingShader.Program, "dirLight.specular");
    // Point light 1
    GLint pPosLoc = glGetUniformLocation(lightingShader.Program, "pointLights[0].position");
    GLint pAmbLoc = glGetUniformLocation(lightingShader.Program, "pointLights[0].ambient");
    GLint pDifLoc = glGetUniformLocation(lightingShader.Program, "pointLights[0].diffuse");
    GLint pSpecLoc = glGetUniformLocation(lightingShader.Program, "pointLights[0].specular");
    GLint pConstLoc = glGetUniformLocation(lightingShader.Program, "pointLights[0].constant");
    GLint pLinLoc = glGetUniformLocation(lightingShader.Program, "pointLights[0].linear");
    GLint pQuadLoc = glGetUniformLocation(lightingShader.Program, "pointLights[0].quadratic");

    // Point light 2
    GLint pPosLoc1 = glGetUniformLocation(lightingShader.Program, "pointLights[1].position");
    GLint pAmbLoc1 = glGetUniformLocation(lightingShader.Program, "pointLights[1].ambient");
    GLint pDifLoc1 = glGetUniformLocation(lightingShader.Program, "pointLights[1].diffuse");
    GLint pSpecLoc1 = glGetUniformLocation(lightingShader.Program, "pointLights[1].specular");
    GLint pConstLoc1 = glGetUniformLocation(lightingShader.Program, "pointLights[1].constant");
    GLint pLinLoc1 = glGetUniformLocation(lightingShader.Program, "pointLights[1].linear");
    GLint pQuadLoc1 = glGetUniformLocation(lightingShader.Program, "pointLights[1].quadratic");

    // Point light 3
    GLint pPosLoc2 = glGetUniformLocation(lightingShader.Program, "pointLights[2].position");
    GLint pAmbLoc2 = glGetUniformLocation(lightingShader.Program, "pointLights[2].ambient");
    GLint pDifLoc2 = glGetUniformLocation(lightingShader.Program, "pointLights[2].diffuse");
    GLint pSpecLoc2 = glGetUniformLocation(lightingShader.Program, "pointLights[2].specular");
    GLint pConstLoc2 = glGetUniformLocation(lightingShader.Program, "pointLights[2].constant");
    GLint pLinLoc2 = glGetUniformLocation(lightingShader.Program, "pointLights[2].linear");
    GLint pQuadLoc2 = glGetUniformLocation(lightingShader.Program, "pointLights[2].quadratic");

    // Point light 4
    GLint pPosLoc3 = glGetUniformLocation(lightingShader.Program, "pointLights[3].position");
    GLint pAmbLoc3 = glGetUniformLocation(lightingShader.Program, "pointLights[3].ambient");
    GLint pDifLoc3 = glGetUniformLocation(lightingShader.Program, "pointLights[3].diffuse");
    GLint pSpecLoc3 = glGetUniformLocation(lightingShader.Program, "pointLights[3].specular");
    GLint pConstLoc3 = glGetUniformLocation(lightingShader.Program, "pointLights[3].constant");
    GLint pLinLoc3 = glGetUniformLocation(lightingShader.Program, "pointLights[3].linear");
    GLint pQuadLoc3 = glGetUniformLocation(lightingShader.Program, "pointLights[3].quadratic");

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
    GLint dayFactorLoc = glGetUniformLocation(lightingShader.Program, "dayFactor");

    // -----------------------------------------------------------------------------
    // LOOP PRINCIPAL DE RENDER
    // -----------------------------------------------------------------------------
    // En cada iteración calculamos deltaTime, procesamos input y actualizamos animaciones,
    // limpiamos el buffer, configuramos view/projection y las luces, dibujamos entorno, bardas, puertas y animales.
    // -----------------------------------------------------------------------------
    while (!window.ShouldClose())
    {
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // -----------------------------------------------------------------
        // Actualizar ciclo día/noche del skybox con el tiempo global
        // y enviar el factor de día al shader de iluminación
        // -----------------------------------------------------------------
        skybox.Update(currentFrame);
        float dayFactor = skybox.GetDayFactor();
        if (dayFactorLoc >= 0)
            glUniform1f(dayFactorLoc, dayFactor);

        window.PollEvents();
        ProcessInput(window);
        Animation();

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // ---- Actualizar posición de la cámara según el modo ----
        if (followCharacter) {
            if (thirdPerson) {
                glm::vec3 offset(
                    -(cameraDistance * sin(glm::radians(personajeRot))),
                    cameraDistance,
                    -(cameraDistance * cos(glm::radians(personajeRot)))
                );
                camera.SetPosition(personajePos + offset);
                camera.LookAt(personajePos + glm::vec3(0.0f, 5.0f, 0.0f));
            }
            else {
                glm::vec3 headPos = personajePos + glm::vec3(0.0f, 3.0f, 0.0f);
                glm::vec3 front(
                    sin(glm::radians(personajeRot)),
                    0.0f,
                    cos(glm::radians(personajeRot))
                );
                camera.SetPosition(headPos + (front * 0.6f));
                camera.LookAt(headPos + front);
            }
        }

        glm::mat4 projection = glm::perspective(
            glm::radians(camera.GetZoom()),
            (float)window.GetBufferWidth() / (float)window.GetBufferHeight(),
            0.1f, 100.0f
        );
        glm::mat4 view = camera.GetViewMatrix();

        // -------------------------------------------------------------------------
        // shader de iluminacion: matrices, camara y configuración de luces
        // -------------------------------------------------------------------------
        lightingShader.Use();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
        glUniform3fv(viewPosLoc, 1, glm::value_ptr(camera.GetPosition()));

        // Luz direccional
        float m = ambLightsOn ? 1.0f : 0.0f;
        glUniform3f(dirDirLoc, -0.2f, -1.0f, -0.3f);
        glUniform3f(dirAmbLoc, 0.5f * m, 0.5f * m, 0.5f * m);
        glUniform3f(dirDifLoc, 0.5f * m, 0.5f * m, 0.5f * m);
        glUniform3f(dirSpecLoc, 0.3f, 0.3f, 0.3f);

        // Punto de luz
        // factor: 1.0 = encendido, 0.0 = apagado
        float k = pointLightsOn ? 1.0f : 0.0f;

        // Punto de luz 1
        glUniform3fv(pPosLoc, 1, glm::value_ptr(pointLightPositions[0]));
        glUniform3f(pAmbLoc, 0.7f * k, 0.7f * k, 0.7f * k);
        glUniform3f(pDifLoc, 0.7f * k, 0.7f * k, 0.7f * k);
        glUniform3f(pSpecLoc, 1.0f * k, 1.0f * k, 1.0f * k);
        glUniform1f(pConstLoc, 1.0f);
        glUniform1f(pLinLoc, 0.04f);
        glUniform1f(pQuadLoc, 0.02f);

        // Punto de luz 2
        glUniform3fv(pPosLoc1, 1, glm::value_ptr(pointLightPositions[1]));
        glUniform3f(pAmbLoc1, 0.7f * k, 0.7f * k, 0.7f * k);
        glUniform3f(pDifLoc1, 0.7f * k, 0.7f * k, 0.7f * k);
        glUniform3f(pSpecLoc1, 1.0f * k, 1.0f * k, 1.0f * k);
        glUniform1f(pConstLoc1, 1.0f);
        glUniform1f(pLinLoc1, 0.04f);
        glUniform1f(pQuadLoc1, 0.02f);

        // Punto de luz 3
        glUniform3fv(pPosLoc2, 1, glm::value_ptr(pointLightPositions[2]));
        glUniform3f(pAmbLoc2, 0.8f * k, 0.8f * k, 0.8f * k);
        glUniform3f(pDifLoc2, 0.8f * k, 0.8f * k, 0.8f * k);
        glUniform3f(pSpecLoc2, 1.0f * k, 1.0f * k, 1.0f * k);
        glUniform1f(pConstLoc2, 1.0f);
        glUniform1f(pLinLoc2, 0.04f);
        glUniform1f(pQuadLoc2, 0.02f);

        // Punto de luz 4
        glUniform3fv(pPosLoc3, 1, glm::value_ptr(pointLightPositions[3]));
        glUniform3f(pAmbLoc3, 0.8f * k, 0.8f * k, 0.8f * k);
        glUniform3f(pDifLoc3, 0.8f * k, 0.8f * k, 0.8f * k);
        glUniform3f(pSpecLoc3, 1.0f * k, 1.0f * k, 1.0f * k);
        glUniform1f(pConstLoc3, 1.0f);
        glUniform1f(pLinLoc3, 0.04f);
        glUniform1f(pQuadLoc3, 0.02f);

        // ---------------------- spotlight (linterna de camara) -------------
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

        // -------------------------------------------------------------------------
        // dibujo del entorno (barda, piso, arco y puertas)
        // -------------------------------------------------------------------------
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

        // -------------------------------------------------------------------------
        // habitat 2 - Ciervo adulto (cuerpo y partes articuladas)
        // -------------------------------------------------------------------------
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




        // ---------------------------------------------------------------------
        // Habitat 2 - Ciervo bebe con cuerpo, cuello, cabeza
        // ---------------------------------------------------------------------
        // ----- CIERVO BEBE -----
        {
            // matriz base identidad el bebe ya viene posicionado desde el .obj
            // y solo se corrige un desplazamiento ligero en X
            glm::mat4 modelCiervoBebe(1.0f);
            // base, colocamos el ciervo en su posición original en el mundo
            modelCiervoBebe = glm::translate(modelCiervoBebe, glm::vec3(1.0f, 0.0f, 0.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelCiervoBebe));
            glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 0);
            // Cuerpo, no tiene rotaciones ni animaciones propias
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

        // ---------------------------------------------------------------------
        // habitat de aves - Pajaro 1 con cuerpo y alas animadas
        // ---------------------------------------------------------------------
        // ----- PAJARO1 -----
        {
            glm::mat4 modelPajaro1(1.0f);
            // posición base del pajaro en el aviario
            // desplazamiento animado (posición en vuelo / idle)
            modelPajaro1 = glm::translate(modelPajaro1, glm::vec3(17.06f, 8.834f, -10.47f));
            modelPajaro1 = glm::translate(modelPajaro1, pajaro1pos);
            modelPajaro1 = glm::rotate(modelPajaro1, glm::radians(pajaro1Rot), glm::vec3(0.0f, 1.0f, 0.0f));
            modelPajaro1 = glm::translate(modelPajaro1, glm::vec3(-17.06f, -8.834f, 10.47f));
            modelTemp = modelPajaro1;
            // Cuerpo
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelPajaro1));
            Pajaro1Body.Draw(lightingShader);

            // Ala derecha
            glm::mat4 model = modelTemp;
            model = glm::translate(model, glm::vec3(16.38f, 9.123f, -10.18f));
            model = glm::rotate(model, glm::radians(ala1), glm::vec3(1.0f, 0.0f, 0.0f));
            model = glm::translate(model, glm::vec3(-16.38f, -9.123f, 10.18f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            Pajaro1AlaDer.Draw(lightingShader);

            //Ala izquierda
            model = modelTemp;
            model = glm::translate(model, glm::vec3(16.66f, 9.126f, -10.98f));
            model = glm::rotate(model, glm::radians(ala2), glm::vec3(1.0f, 0.0f, 0.0f));
            model = glm::translate(model, glm::vec3(-16.66f, -9.126f, 10.98f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            Pajaro1AlaIzq.Draw(lightingShader);
        }

        // ---------------------------------------------------------------------
        // habitat de aves - Pájaro 2 con cuerpo, cabeza y cola
        // ---------------------------------------------------------------------
        // ----- PAJARO2 -----
        {
            glm::mat4 modelPajaro2(1.0f);
            // posicion base del pajaro en la rama
            modelPajaro2 = glm::translate(modelPajaro2, glm::vec3(22.13f, 10.55f, -16.83f));
            // desplazamiento animado
            modelPajaro2 = glm::translate(modelPajaro2, pajaro2pos);
            modelPajaro2 = glm::rotate(modelPajaro2, glm::radians(pajaro2Rot), glm::vec3(0.0f, 1.0f, 0.0f));
            modelPajaro2 = glm::translate(modelPajaro2, glm::vec3(-22.13f, -10.55f, 16.83f));
            modelTemp = modelPajaro2;
            // Cuerpo
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelPajaro2));
            Pajaro2Body.Draw(lightingShader);

            //cabeza
            glm::mat4 model = modelTemp;
            model = glm::translate(model, glm::vec3(22.22f, 11.0f, -16.87f));
            model = glm::rotate(model, glm::radians(pajaro2head), glm::vec3(0.0f, 0.0f, 1.0f));
            model = glm::translate(model, glm::vec3(-22.22f, -11.0f, 16.87f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            Pajaro2Head.Draw(lightingShader);

            // cola
            model = modelTemp;
            model = glm::translate(model, glm::vec3(21.11f, 9.278f, -17.48f));
            model = glm::rotate(model, glm::radians(pajaro2tail), glm::vec3(0.0f, 0.0f, 1.0f));
            model = glm::translate(model, glm::vec3(-21.11f, -9.278f, 17.48f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            Pajaro2Tail.Draw(lightingShader);
        }

        // ---------------------------------------------------------------------
        // habitat de aves - Tucan, pajaro 3 con cuerpo, cabeza, pico y cola
        // ---------------------------------------------------------------------
        // ----- TUCAN (PAJARO3) -----
        {
            glm::mat4 modelPajaro3(1.0f);
            modelPajaro3 = glm::translate(modelPajaro3, glm::vec3(-0.27f, 0.0f, 0.0f));
            modelPajaro3 = glm::translate(modelPajaro3, glm::vec3(11.55f, 12.56f, -10.18f));
            modelPajaro3 = glm::translate(modelPajaro3, pajaro3pos);
            // rotacion global
            modelPajaro3 = glm::rotate(modelPajaro3, glm::radians(pajaro3Rot), glm::vec3(0.0f, 1.0f, 0.0f));
            modelPajaro3 = glm::translate(modelPajaro3, glm::vec3(-11.55f, -12.56f, 10.18f));
            // Cuerpo
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelPajaro3));
            modelTemp = modelPajaro3;
            TucanBody.Draw(lightingShader);
            //cabeza
            glm::mat4 model1 = modelTemp;
            model1 = modelTemp;
            model1 = glm::translate(model1, glm::vec3(11.64f, 13.74f, -10.81f));
            model1 = glm::rotate(model1, glm::radians(pajaro3head), glm::vec3(0.0f, 0.0f, 1.0f));
            model1 = glm::translate(model1, glm::vec3(-11.64f, -13.74f, 10.81f));
            TucanHead.Draw(lightingShader);
            //pico superior
            modelTemp2 = model1;
            model1 = modelTemp2;
            model1 = glm::translate(model1, glm::vec3(11.34, 13.64f, -10.06f));
            model1 = glm::rotate(model1, glm::radians(pico1), glm::vec3(-1.0f, 0.0f, 0.0f));
            model1 = glm::translate(model1, glm::vec3(-11.34, -13.64f, 10.06f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model1));
            TucanPico1.Draw(lightingShader);
            //pico inferior
            model1 = modelTemp2;
            model1 = glm::translate(model1, glm::vec3(11.34, 13.64f, -10.06f));
            model1 = glm::rotate(model1, glm::radians(pico2), glm::vec3(1.0f, 0.0f, 0.0f));
            model1 = glm::translate(model1, glm::vec3(-11.34, -13.64f, 10.06f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model1));
            TucanPico2.Draw(lightingShader);
            //cola
            glm::mat4 model = modelTemp;
            model = glm::translate(model, glm::vec3(11.42f, 12.48f, -12.66f));
            model = glm::rotate(model, glm::radians(pajaro3tail), glm::vec3(1.0f, 0.0f, 0.0f));
            model = glm::translate(model, glm::vec3(-11.42f, -12.48f, 12.66f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            TucanTail.Draw(lightingShader);
        }

        // ---------------------------------------------------------------------
        // elementos del entorno como rejas, cajas, arbustos, pinos, aviario
        // ---------------------------------------------------------------------
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
        // ----- ARBOL CENTRAL DEL AVIARIO -----
        {
            glm::mat4 model(1.0f);
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            ArbolAv.Draw(lightingShader);
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            HojasAv.Draw(lightingShader);
        }
        // ----- AVIARIO (opaco) -----
        {
            glm::mat4 model(1.0f);
            glUniform1i(transpLoc, 0);
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            Aviario.Draw(lightingShader);
        }




        // ---------------------------------------------------------------------
        // aviario - vidrio transparente
        // ---------------------------------------------------------------------
        // ----- AVIARIO VIDRIO (transparente) -----
        {
            glm::mat4 modelAv(1.0f);
            // activamos blending para simular el vidrio
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelAv));
            glUniform1f(glGetUniformLocation(lightingShader.Program, "alpha"), 0.5f);
            glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 1);
            AviarioV.Draw(lightingShader);
            // restauramos estado por defecto
            glDisable(GL_BLEND);
            glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 0);
            glUniform1f(glGetUniformLocation(lightingShader.Program, "alpha"), 1.0f);
        }

        // ---------------------------------------------------------------------
        // props decorativos flores y banca
        // ---------------------------------------------------------------------
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

        // ---------------------------------------------------------------------
        // habitat polar - Pinguino animado
        // ---------------------------------------------------------------------
        // ----- PINGUINO -----
        {
            glm::mat4 modelPingu(1.0f);

            // Base para Hábitat Polar (ajustada para que Pinguino, Oso, y Foca estén separados)
            glm::vec3 habitat_polar_center(22.0f, 0.0f, 8.0f);
            // posicion base del pinguino en el habitat
            modelPingu = glm::translate(modelPingu, habitat_polar_center);
            // Movimiento ping-pong a lo largo del camino
            modelPingu = glm::translate(modelPingu, pinguPos);
            modelPingu = glm::rotate(modelPingu, glm::radians(pinguRot + 360.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            // Escala general
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

        // ---------------------------------------------------------------------
        // habitat marino - Piraña con cabeza y cola animadas
        // ---------------------------------------------------------------------
        // ----- PIRAÑA ------
        {
            glm::mat4 modelPira(1.0f);
            glm::vec3 habitat_marino_center(-24.0f, 2.0f, -16.0f);
            // posicion base de la pecera
            modelPira = glm::translate(modelPira, habitat_marino_center);
            modelPira = glm::translate(modelPira, piranhaPos);
            // rotacion según direccion
            modelPira = glm::rotate(modelPira, glm::radians(piranhaRot), glm::vec3(0.0f, 1.0f, 0.0f));
            modelPira = glm::scale(modelPira, glm::vec3(0.5f));

            // Dibuja cabeza con pivote ajustado según modelo
            glm::mat4 modelHead = modelPira;
            const glm::vec3 HEAD_PIVOT(-0.002055f, 0.885469f, -1.231676f);
            modelHead = glm::translate(modelHead, HEAD_PIVOT);
            modelHead = glm::rotate(modelHead, glm::radians(piraHeadAngle), glm::vec3(0, 1, 0));
            modelHead = glm::translate(modelHead, -HEAD_PIVOT);
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelHead));
            piranhaH.Draw(lightingShader);
            // ----- Cola -----
            glm::mat4 modelTail = modelPira;
            const glm::vec3 TAIL_PIVOT(-0.001568f, 0.824135f, -1.087399f);
            modelTail = glm::translate(modelTail, TAIL_PIVOT);
            modelTail = glm::rotate(modelTail, glm::radians(piraTailAngle), glm::vec3(0, 1, 0));
            modelTail = glm::translate(modelTail, -TAIL_PIVOT);
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelTail));
            piranhaT.Draw(lightingShader);
        }

        // ---------------------------------------------------------------------
        // habitat polar - Foca con cuerpo, cabeza, aletas y cola)
        // ---------------------------------------------------------------------
        // ------- FOCA ------
        {
            glm::mat4 base(1.0f);
            glm::vec3 habitat_polar_center(14.0f, 0.55f, 13.0f);
            // posicion base de la foca
            base = glm::translate(base, habitat_polar_center);
            base = glm::translate(base, sealPos);                      // Movimiento ping-pong
            base = glm::rotate(base, glm::radians(sealRot - 90.0f), glm::vec3(0, 1, 0));
            base = glm::scale(base, glm::vec3(2.5f));

            // ----- Cuerpo -----
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(base));
            sealB.Draw(lightingShader);

            // ----- Cabeza
            {
                glm::mat4 m = base;
                const glm::vec3 HEAD_PIVOT(0.0f, 0.35f, 0.7f);
                m = glm::translate(m, HEAD_PIVOT);
                m = glm::rotate(m, glm::radians(sealHead_A), glm::vec3(1.0f, 0.0f, 0.0f));
                m = glm::translate(m, -HEAD_PIVOT);
                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
                sealH.Draw(lightingShader);
            }

            // ----- Manos / aletas delanteras
            {
                glm::mat4 m = base;
                const glm::vec3 HANDS_PIVOT(0.0f, 0.15f, 0.3f);
                m = glm::translate(m, HANDS_PIVOT);
                m = glm::rotate(m, glm::radians(sealHands_A), glm::vec3(1.0f, 0.0f, 0.0f));
                m = glm::translate(m, -HANDS_PIVOT);
                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
                sealHS.Draw(lightingShader);
            }
            // ----- Cola con un ligero movimiento
            {
                glm::mat4 m = base;
                const glm::vec3 TAIL_PIVOT(0.0f, 0.1f, -0.6f);
                float tail_A = std::sin(sealTime * 1.5f) * 8.0f;
                m = glm::translate(m, TAIL_PIVOT);
                m = glm::rotate(m, glm::radians(tail_A), glm::vec3(0.0f, 1.0f, 0.0f));
                m = glm::translate(m, -TAIL_PIVOT);
                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
                sealT.Draw(lightingShader);
            }
        }

        // ---------------------------------------------------------------------
        // habitat polar - Oso polar con cuerpo y patas articuladas
        // ---------------------------------------------------------------------
        // ------ OSO POLAR -------
        {
            glm::mat4 base(1.0f);
            glm::vec3 habitat_polar_center(18.0f, 0.0f, 17.0f);
            // posicion base del oso
            base = glm::translate(base, habitat_polar_center);
            base = glm::translate(base, pBearPos);                       // Movimiento ping-pong
            base = glm::rotate(base, glm::radians(pBearRot - 90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            base = glm::scale(base, glm::vec3(2.5f));

            // ----- Cuerpo -----
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(base));
            pBearBody.Draw(lightingShader);

            // ----- Cabeza -----
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(base));
            pBearHead.Draw(lightingShader);

            // ----- Pivotes dadas en coordenadas LOCALES del oso -----
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

        // ---------------------------------------------------------------------
        // habitat marino - Tiburon con cuerpo, cabeza y cola)
        // ---------------------------------------------------------------------
        // ------- TIBURON ----
        {
            glm::mat4 modelTibu(1.0f);
            glm::vec3 habitat_marino_center(-20.0f, 0.0f, -19.0f);
            // Posicion base del tiburon en el acuario
            modelTibu = glm::translate(modelTibu, habitat_marino_center);
            modelTibu = glm::translate(modelTibu, sharkPos);
            modelTibu = glm::rotate(modelTibu, glm::radians(sharkRot), glm::vec3(0.0f, 1.0f, 0.0f));
            modelTibu = glm::scale(modelTibu, glm::vec3(0.05f));
            // oscilacion suave del cuerpo, representa nado
            glm::mat4 modelBody = glm::rotate(modelTibu, glm::radians(sharkBodyAngle), glm::vec3(0.0f, 1.0f, 0.0f));
            // Dibuja el cuerpo
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelBody));
            sharkB.Draw(lightingShader);
            //cabeza
            glm::mat4 modelHead = modelBody;
            const glm::vec3 HEAD_PIVOT(0.0f, 30.8f, -1.2f);
            modelHead = glm::translate(modelHead, HEAD_PIVOT);
            modelHead = glm::rotate(modelHead, glm::radians(sharkHeadAngle), glm::vec3(0.0f, 1.0f, 0.0f));
            modelHead = glm::translate(modelHead, -HEAD_PIVOT);
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelHead));
            sharkH.Draw(lightingShader);
            //cola
            glm::mat4 modelCola = modelBody;
            const glm::vec3 TAIL_PIVOT(0.0f, 27.05f, -46.71f);
            modelCola = glm::translate(modelCola, TAIL_PIVOT);
            modelCola = glm::rotate(modelCola, glm::radians(sharkTailA), glm::vec3(0.0f, 1.0f, 0.0f));
            modelCola = glm::translate(modelCola, -TAIL_PIVOT);
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelCola));
            sharkT.Draw(lightingShader);
        }

        // ---------------------------------------------------------------------
        // props habitat polar - Iglu y barreras que son de metal y vidrio
        // ---------------------------------------------------------------------
        // ----- IGLU -----
        {
            glm::mat4 modelIglu(1.0f);

            // Posición del iglú
            modelIglu = glm::translate(modelIglu, glm::vec3(5.0f, 0.0f, 7.0f)); // Posicion relativa al centro polar
            modelIglu = glm::scale(modelIglu, glm::vec3(0.8f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelIglu));
            glUniform1i(transpLoc, 0);      // Iglú opaco
            Iglu.Draw(lightingShader);
        }

        // ----- AREA CONTENIDA DEL HABITAT POLAR
        {
            glm::mat4 model(1.0f);
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            BardaMetal.Draw(lightingShader);
        }
        {
            glm::mat4 modelVidrio(1.0f);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelVidrio));
            glUniform1f(glGetUniformLocation(lightingShader.Program, "alpha"), 0.3f);
            glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 1);
            BardaVidrio.Draw(lightingShader);
            // Restaurar estado por defecto
            glDisable(GL_BLEND);
            glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 0);
            glUniform1f(glGetUniformLocation(lightingShader.Program, "alpha"), 1.0f);
        }


        // ---------------------------------------------------------------------
        // habitat marino - Arena, corales, agua y pecera
        // ---------------------------------------------------------------------
        // ----- AREA CONTENIDA DE HABITAT MARINO
        {
            glm::mat4 model(1.0f);
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            Arena.Draw(lightingShader);
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            Coral1.Draw(lightingShader);
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            Coral2.Draw(lightingShader);
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            Coral3.Draw(lightingShader);
        }
        // ----- Agua del acuario transparente -----
        {
            glm::mat4 modelAgua(1.0f);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelAgua));
            glUniform1f(glGetUniformLocation(lightingShader.Program, "alpha"), 0.75f);
            glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 1);
            Agua.Draw(lightingShader);
            // restaurar estado por defecto
            glDisable(GL_BLEND);
            glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 0);
            glUniform1f(glGetUniformLocation(lightingShader.Program, "alpha"), 1.0f);
        }
        // ----- Pecera vidrio del tanque marino -----

        {
            glm::mat4 modelPecera(1.0f);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelPecera));
            glUniform1f(glGetUniformLocation(lightingShader.Program, "alpha"), 0.5f);
            glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 1);
            Pecera.Draw(lightingShader);
            glDisable(GL_BLEND);
            glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 0);
            glUniform1f(glGetUniformLocation(lightingShader.Program, "alpha"), 1.0f);
        }
        // ----- BANQUITAS -----
        {
            glm::mat4 model(1.0f);
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            Banquitas.Draw(lightingShader);
        }

        // ---------------------------------------------------------
        // PERSONAJE PRINCIPAL COMPLETO (multipartes)
        // ---------------------------------------------------------
        {
            // Transformación base del personaje (posición + rotación + escala)
            glm::mat4 base = glm::mat4(1.0f);
            base = glm::translate(base, personajePos);
            base = glm::rotate(
                base,
                glm::radians(personajeRot),
                glm::vec3(0.0f, 1.0f, 0.0f)
            );
            base = glm::scale(base, glm::vec3(0.004f));   // ajusta si queda muy grande/pequeño

            // ----- TORSO -----
            glm::mat4 torsoM = base;
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(torsoM));
            P_Torso.Draw(lightingShader);

            // ----- CABEZA -----
            glm::mat4 headM = base;
            headM = glm::translate(headM, glm::vec3(0.0f, 50.0f, 0.0f)); // offset vertical de la cabeza
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(headM));
            P_Cabeza.Draw(lightingShader);

            // ----- BRAZO DERECHO -----
            glm::mat4 brazoDR = base;
            // posición del hombro derecho (ajusta estos valores según tu modelo)
            brazoDR = glm::translate(brazoDR, glm::vec3(-20.0f, 40.0f, 0.0f));
            // rotación de péndulo (caminar) en eje X
            brazoDR = glm::rotate(
                brazoDR,
                glm::radians(armRightAngle),
                glm::vec3(1.0f, 0.0f, 0.0f)
            );
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(brazoDR));
            P_BrazoDer.Draw(lightingShader);

            // ----- BRAZO IZQUIERDO -----
            glm::mat4 brazoIZ = base;
            brazoIZ = glm::translate(brazoIZ, glm::vec3(20.0f, 40.0f, 0.0f));
            brazoIZ = glm::rotate(
                brazoIZ,
                glm::radians(armLeftAngle),
                glm::vec3(1.0f, 0.0f, 0.0f)
            );
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(brazoIZ));
            P_BrazoIzq.Draw(lightingShader);

            // ----- PIERNA DERECHA -----
            glm::mat4 piernaDR = base;
            // posición de la cadera derecha
            piernaDR = glm::translate(piernaDR, glm::vec3(-10.0f, 10.0f, 0.0f));
            piernaDR = glm::rotate(
                piernaDR,
                glm::radians(legRightAngle),
                glm::vec3(1.0f, 0.0f, 0.0f)
            );
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(piernaDR));
            P_PiernaDer.Draw(lightingShader);

            // ----- PIERNA IZQUIERDA -----
            glm::mat4 piernaIZ = base;
            piernaIZ = glm::translate(piernaIZ, glm::vec3(10.0f, 10.0f, 0.0f));
            piernaIZ = glm::rotate(
                piernaIZ,
                glm::radians(legLeftAngle),
                glm::vec3(1.0f, 0.0f, 0.0f)
            );
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(piernaIZ));
            P_PiernaIzq.Draw(lightingShader);
        }



        // ---------------------------------------------------------------------
        // Mobiliario de iluminaciOn - Postes y lámparas (cubo + poste)
        // ---------------------------------------------------------------------
        // Cada par de "Cubo 1 / Cubo 2" corresponde a una lámpara de pie:
        //   - Cubo 1: lámpara caja superior con textura lampTexture.
        //   - Cubo 2: poste de madera, caja alargada con textura maderaTextura
        // Lampara 1 lado derecho frente al aviario
        // Cubo 1 - lampara
        {
            // textura difusa para el cubo
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, lampTexture);
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(23.0f, 7.0f, 0.0f)); // posición en tu escena
            model = glm::scale(model, glm::vec3(2.5f));                  // tamaño
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glBindVertexArray(cubeVAO);
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
        }

        // Cubo 2 poste de madera
        {
            // textura difusa para el cubo
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, maderaTextura);
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(23.0f, 3.5f, 0.0f));
            model = glm::scale(model, glm::vec3(1.0f, 7.0f, 1.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glBindVertexArray(cubeVAO);
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
        }

        // Lampara 2, lado izquierdo cerca del marino/polar
        // Cubo 1 - lampara
        {
            // textura difusa para el cubo
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, lampTexture);
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(-23.0f, 7.0f, 2.0f));
            model = glm::scale(model, glm::vec3(2.5f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glBindVertexArray(cubeVAO);
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
        }

        // Cubo 2 poste
        {
            // textura difusa para el cubo
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, maderaTextura);
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(-23.0f, 3.5f, 2.0f));
            model = glm::scale(model, glm::vec3(1.0f, 7.0f, 1.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glBindVertexArray(cubeVAO);
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
        }

        // Lampara, zona trasera del zool
        // Cubo 1 – lampara
        {
            // textura difusa para el cubo
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, lampTexture);
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(-1.0f, 7.0f, -23.0f));
            model = glm::scale(model, glm::vec3(2.5f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glBindVertexArray(cubeVAO);
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
        }

        // Cubo 2 poste
        {
            // textura difusa para el cubo
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, maderaTextura);
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(-1.0f, 3.0f, -23.0f));
            model = glm::scale(model, glm::vec3(1.0f, 7.0f, 1.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glBindVertexArray(cubeVAO);
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
        }

        // Lampara 4, zona frontal central
        // Cubo 1 - lámpara
        {
            // textura difusa para el cubo
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, lampTexture);
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(-1.0f, 7.0f, 7.0f));
            model = glm::scale(model, glm::vec3(2.5f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glBindVertexArray(cubeVAO);
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
        }

        // Cubo 2 poste
        {
            // textura difusa para el cubo
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, maderaTextura);
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(-1.0f, 3.0f, 7.0f));
            model = glm::scale(model, glm::vec3(1.0f, 7.0f, 1.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glBindVertexArray(cubeVAO);
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
        }

        // ---------------------------------------------------------------------
        // carteles informativos conformados por cubo aplanado + poste
        // ---------------------------------------------------------------------
        // CARTELES - Habitat 2 ciervos
        {
            // textura difusa para el cubo
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, cartelTextura);
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(-4.0f, 4.0f, 10.0f));
            model = glm::scale(model, glm::vec3(0.4f, 3.0f, 3.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glBindVertexArray(cubeVAO);
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
        }
        // Poste del cartel (madera)
        {
            // textura difusa para el cubo
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, maderaTextura);
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(-4.0f, 1.0f, 10.0f));
            model = glm::scale(model, glm::vec3(0.5f, 3.0f, 0.5f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glBindVertexArray(cubeVAO);
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
        }
        // CARTELES - habitat 4 (oso polar, pinguino, foca)
        {
            // textura difusa para el cubo
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, cartelTextura1);
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(2.0f, 4.0f, 10.0f));
            model = glm::scale(model, glm::vec3(0.4f, 3.0f, 3.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glBindVertexArray(cubeVAO);
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
        }





        // Poste del cartel polar (madera)
        // Cubo 2
        {
            // textura difusa para el cubo
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, maderaTextura);
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(2.0f, 1.0f, 10.0f));
            model = glm::scale(model, glm::vec3(0.5f, 3.0f, 0.5f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glBindVertexArray(cubeVAO);
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
        }
        // CARTELES - Habitat 1 (acuario / marino)
        {
            // textura difusa para el cubo
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, cartelTextura2);
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(-6.0f, 4.0f, -8.0f));
            model = glm::scale(model, glm::vec3(0.4f, 3.0f, 3.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glBindVertexArray(cubeVAO);
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
        }
        // Poste del cartel del acuario
        // Cubo 2
        {
            // textura difusa para el cubo
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, maderaTextura);
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(-6.0f, 1.0f, -8.0f));
            model = glm::scale(model, glm::vec3(0.5f, 3.0f, 0.5f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glBindVertexArray(cubeVAO);
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
        }

        // ---------------------------------------------------------------------
        // skybox, entorno cubico alrededor de toda la escena
        // ---------------------------------------------------------------------
        skybox.Render(view, projection);

        window.SwapBuffers();
    }

    return 0;
}

// ============================================================================
//  ProcessInput
//  Descripcion:
//      Lee el estado del teclado y mouse desde la clase Window y actualiza:
//        - Movimiento de la cámara (W, A, S, D, flechas, Z, X).
//        - Rotación de la cámara con el mouse.
//        - Toggles de animaciones por hábitat y luces (P, U, B, C, Y, L, O).
// ============================================================================
void ProcessInput(Window& window)
{
    // --- Cámara libre (solo cuando NO sigue al personaje) ---
    if (!followCharacter) {
        if (window.IsKeyPressed(GLFW_KEY_W))
            camera.ProcessKeyboard(FORWARD, deltaTime);
        if (window.IsKeyPressed(GLFW_KEY_S))
            camera.ProcessKeyboard(BACKWARD, deltaTime);
        if (window.IsKeyPressed(GLFW_KEY_A))
            camera.ProcessKeyboard(LEFT, deltaTime);
        if (window.IsKeyPressed(GLFW_KEY_D))
            camera.ProcessKeyboard(RIGHT, deltaTime);

        if (window.IsKeyPressed(GLFW_KEY_Z))
            camera.ProcessKeyboard(UP, deltaTime);
        if (window.IsKeyPressed(GLFW_KEY_X))
            camera.ProcessKeyboard(DOWN, deltaTime);
    }

    // --- Rotación de cámara con el mouse ---
    float xOffset = window.GetXChange();
    float yOffset = window.GetYChange();
    if (xOffset != 0.0f || yOffset != 0.0f) {
        camera.ProcessMouseMovement(xOffset, yOffset);
    }

    bool moving = false;  // <--- NUEVO

    // --- Movimiento del personaje principal con flechas ---
    if (window.IsKeyPressed(GLFW_KEY_UP)) {
        float velocity = moveSpeed * deltaTime;
        personajePos += glm::vec3(
            sin(glm::radians(personajeRot)) * velocity,
            0.0f,
            cos(glm::radians(personajeRot)) * velocity
        );
        moving = true;   
    }

    if (window.IsKeyPressed(GLFW_KEY_DOWN)) {
        float velocity = moveSpeed * deltaTime;
        personajePos -= glm::vec3(
            sin(glm::radians(personajeRot)) * velocity,
            0.0f,
            cos(glm::radians(personajeRot)) * velocity
        );
        moving = true;  
    }

    if (window.IsKeyPressed(GLFW_KEY_LEFT)) {
        personajeRot += turnSpeed * deltaTime;
    }

    if (window.IsKeyPressed(GLFW_KEY_RIGHT)) {
        personajeRot -= turnSpeed * deltaTime;
    }

    // al final del bloque de movimiento:
    isWalking = moving;

    // --- Alternar modo de cámara: Q (1ª / 3ª persona) ---
    static bool QPressed = false;
    bool nowQ = window.IsKeyPressed(GLFW_KEY_Q);
    if (nowQ && !QPressed) {
        thirdPerson = !thirdPerson;
        QPressed = true;
    }
    else if (!nowQ) {
        QPressed = false;
    }

    // --- Alternar seguir personaje / cámara libre: E ---
    static bool EPressed = false;
    bool nowE = window.IsKeyPressed(GLFW_KEY_E);
    if (nowE && !EPressed) {
        followCharacter = !followCharacter;
        EPressed = true;
    }
    else if (!nowE) {
        EPressed = false;
    }

    // --- Puerta (tecla P) ---
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

    // --- Hábitat 1 (acuático) - tecla U ---
    bool uNow = window.IsKeyPressed(GLFW_KEY_U);
    if (uNow && !animacionH1TogglePressed) {
        animacionH1 = !animacionH1;
        animacionH1TogglePressed = true;
    }
    else if (!uNow) {
        animacionH1TogglePressed = false;
    }

    // --- Hábitat 2 (ciervos) - tecla B ---
    bool bNow = window.IsKeyPressed(GLFW_KEY_B);
    if (bNow && !animacionH2TogglePressed)
    {
        animacionH2 = !animacionH2;
        animacionH2TogglePressed = true;
    }
    else if (!bNow)
    {
        animacionH2TogglePressed = false;
    }

    // --- Pájaros (aviario) - tecla C ---
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

    // --- Hábitat 4 (Oso, Pingüino, Foca) - tecla Y ---
    bool yNow = window.IsKeyPressed(GLFW_KEY_Y);
    if (yNow && !animacionH4TogglePressed) {
        animacionH4 = !animacionH4;
        animacionH4TogglePressed = true;
    }
    else if (!yNow) {
        animacionH4TogglePressed = false;
    }

    // ---------------------------------------------------------
    // control de luces
    // ---------------------------------------------------------
    bool lNow = window.IsKeyPressed(GLFW_KEY_L);

    if (lNow && !pointLightsTogglePressed)
    {
        pointLightsOn = !pointLightsOn;      // cambia de encendido <-> apagado
        pointLightsTogglePressed = true;
    }
    else if (!lNow)
    {
        pointLightsTogglePressed = false;    // suelta la tecla para poder volver a togglear
    }

    bool oNow = window.IsKeyPressed(GLFW_KEY_O);

    if (oNow && !ambLightsTogglePressed)
    {
        ambLightsOn = !ambLightsOn;      // cambia de encendido <-> apagado
        ambLightsTogglePressed = true;
    }
    else if (!oNow)
    {
        ambLightsTogglePressed = false;    // suelta la tecla para poder volver a togglear
    }
}






// ============================================================================
//  Funcio Animation
//  Descripcion:
//      Actualiza, en cada frame, el estado de las animaciones del zoológico:
//        - Apertura/cierre de la puerta principal.
//        - Ciervo adulto: patas, cabeza y recorrido ping-pong en su hábitat.
//        - Ciervo bebé: animación cíclica de cuello y cabeza.
//        - Pájaros en el aviario: pájaro 1 (vuelo), pájaro 2 y tucán (rotaciones).
//      Esta función NO dibuja nada, solo modifica variables globales
//      que luego son usadas en el render dentro del loop principal.
// ============================================================================
void Animation()
{
    // Puerta
    if (puertaAbierta)
    {
        if (rotPuerta < 90.0f) rotPuerta += velocidadPuerta;
    }
    else
    {
        if (rotPuerta > 0.0f) rotPuerta -= velocidadPuerta;
    }

    // Ciervo Adulto (Movimiento y Patas y Ciervo Bebe
    if (animacionH2)
    {
        //Animación de patas Ciervo Adulto
        if (!Ciervostep) {
            RLegs += 0.3f;
            FLegs += 0.3f;
            head += 0.3f;
            if (RLegs > 15.0f) Ciervostep = true;
        }
        else {
            RLegs -= 0.3f;
            FLegs -= 0.3f;
            head -= 0.3f;
            if (RLegs < -15.0f) Ciervostep = false;
        }

        // 2) Movimiento Ciervo Adulto
        if (!ciervoTurning) {
            // Caminar recto
            float dz = ciervoStep * ciervoDir;
            ciervoPos.z += dz;
            ciervoMoved += fabsf(dz);
            if (ciervoMoved >= ciervoSegLen) {
                ciervoMoved = 0.0f;
                ciervoTurning = true;
                ciervoTurnLeft = 180.0f;
            }
        }
        else {
            float dYaw = std::min(ciervoTurnSpeed, ciervoTurnLeft);
            ciervoRot += dYaw;
            ciervoTurnLeft -= dYaw;

            if (ciervoTurnLeft <= 0.0f) {
                ciervoTurning = false;
                ciervoDir *= -1;
            }

            if (ciervoRot >= 360.0f) ciervoRot -= 360.0f;
            if (ciervoRot < 0.0f) ciervoRot += 360.0f;
        }

        // Ciervo Bebé
        const float NECK_SPEED = 0.3f;
        const float HEAD_SPEED = 0.3f;

        const float NECK_MAX = 16.0f;
        const float HEAD_LIM = 15.0f;

        // estado persistente
        static int fase = 0;   // 0: bajar cuello, 1: cabecear, 2: subir cuello
        static int dir = -1;  // -1 baja cabeza, +1 sube
        static int reps = 0;

        if (fase == 0) { // bajar cuello hacia +20
            if (neck < NECK_MAX) neck += NECK_SPEED;
            else { neck = NECK_MAX; head2 = 0.0f; dir = -1; reps = 0; fase = 1; }
        }
        else if (fase == 1) {
            head2 += (dir * HEAD_SPEED);
            if (head2 <= -HEAD_LIM) { head2 = -HEAD_LIM; dir = +1; }
            if (head2 >= +HEAD_LIM) { head2 = +HEAD_LIM; dir = -1; reps++; }
            if (reps >= 2) fase = 2;
        }
        else { // subir cuello a 0 y centrar cabeza
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
        //Vuelve a neutro si se apaga la animación
            // 1)  patas y cabeza a 0
        if (RLegs > 0.0f)      RLegs -= 0.3f;
        else if (RLegs < 0.0f) RLegs += 0.3f;

        if (FLegs > 0.0f)      FLegs -= 0.3f;
        else if (FLegs < 0.0f) FLegs += 0.3f;

        if (head > 0.0f)       head -= 0.3f;
        else if (head < 0.0f)  head += 0.3f;

        // 2) Volver posición XYZ al punto inicial
        if (ciervoPos.x < ciervoHomePos.x)      ciervoPos.x += ciervoReturnPosStep;
        else if (ciervoPos.x > ciervoHomePos.x) ciervoPos.x -= ciervoReturnPosStep;

        if (ciervoPos.y < ciervoHomePos.y)      ciervoPos.y += ciervoReturnPosStep;
        else if (ciervoPos.y > ciervoHomePos.y) ciervoPos.y -= ciervoReturnPosStep;

        if (ciervoPos.z < ciervoHomePos.z)      ciervoPos.z += ciervoReturnPosStep;
        else if (ciervoPos.z > ciervoHomePos.z) ciervoPos.z -= ciervoReturnPosStep;

        // 3) Volver rotación al ángulo original
        if (ciervoRot < ciervoHomeRot)      ciervoRot += ciervoReturnRotStep;
        else if (ciervoRot > ciervoHomeRot) ciervoRot -= ciervoReturnRotStep;

        if (fabs(ciervoPos.x - ciervoHomePos.x) < 0.001f) ciervoPos.x = ciervoHomePos.x;
        if (fabs(ciervoPos.y - ciervoHomePos.y) < 0.001f) ciervoPos.y = ciervoHomePos.y;
        if (fabs(ciervoPos.z - ciervoHomePos.z) < 0.001f) ciervoPos.z = ciervoHomePos.z;
        if (fabs(ciervoRot - ciervoHomeRot) < 0.5f)   ciervoRot = ciervoHomeRot;

        //limpia estados
        bool atHomePos = fabs(ciervoPos.x - ciervoHomePos.x) < 0.001f &&
            fabs(ciervoPos.y - ciervoHomePos.y) < 0.001f &&
            fabs(ciervoPos.z - ciervoHomePos.z) < 0.001f;
        bool atHomeRot = fabs(ciervoRot - ciervoHomeRot) < 0.5f;

        if (atHomePos && atHomeRot) {
            ciervoTurning = false;
            ciervoTurnLeft = 0.0f;
            ciervoMoved = 0.0f;
            ciervoDir = 1;
        }

        // Ciervo Bebé
        const float S1 = 0.6f, S2 = 0.8f;
        if (neck > 0.0f) neck = std::max(0.0f, neck - S1);
        if (head2 > 0.0f) head2 = std::max(0.0f, head2 - S2);
        if (head2 < 0.0f) head2 = std::min(0.0f, head2 + S2);
    }


    // ===== Animación Pájaros (H3) =====
    // Pajaro 1
    if (pajaro1Anim) {
        pajaroPhase += flyHz;
        pajaro1pos.x = flyAmpX * sinf(pajaroPhase);

        pajaro1pos.y = flyHoverY + flyAmpY * sinf(pajaroPhase * 1.3f);
        pajaro1Rot = flyYawAmp * sinf(pajaroPhase);

        // Aleteo
        float wingPhase = pajaroPhase * (wingHz / std::max(0.0001f, flyHz));
        ala1 = wingAmp * sinf(wingPhase);   // derecha
        ala2 = -wingAmp * sinf(wingPhase);   // izquierda
    }
    else {
        auto go0 = [](float& v, float s) {
            if (v > 0) v = std::max(0.0f, v - s);
            else if (v < 0) v = std::min(0.0f, v + s);
            };
        go0(pajaro1pos.x, 0.05f);
        go0(pajaro1pos.y, 0.05f);
        go0(pajaro1Rot, 0.6f);
        go0(ala1, 2.0f);
        go0(ala2, 2.0f);
    }

    // Pajaro 2: rotación simple
    if (pajaro2Anim) {
        if (!pajaro2step) {
            pajaro2head += 0.3f;
            pajaro2Rot += 0.5f;
            pajaro2tail += 1.0f;
            if (pajaro2Rot > 20.0f) pajaro2step = true;
        }
        else {
            pajaro2head -= 0.3f;
            pajaro2Rot -= 0.5f;
            pajaro2tail -= 1.0f;
            if (pajaro2Rot < -20.0f) pajaro2step = false;
        }
    }
    else {
        // Vuelve a neutro
        pajaro2head = 0.0f;
        pajaro2Rot = 0.0f;
        pajaro2tail = 0.0f;
        pajaro2step = false;
    }

    // Tucán
    if (pajaro3Anim) {
        if (!pajaro3step) {
            pajaro3head += 0.3f;
            pajaro3Rot += 0.4f;
            pajaro3tail += 1.0f;

            pico1 += picoVel;
            pico2 += picoVel;

            // límite superior (no abre más que picoMax)
            if (pico1 > picoMax) pico1 = picoMax;
            if (pico2 > picoMax) pico2 = picoMax;

            if (pajaro3Rot > 20.0f) pajaro3step = true;
        }
        else {
            pajaro3head -= 0.3f;
            pajaro3Rot -= 0.4f;
            pajaro3tail -= 1.0f;

            pico1 -= picoVel;
            pico2 -= picoVel;

            // límite inferior (no se pasa de cerrado)
            if (pico1 < 0.0f) pico1 = 0.0f;
            if (pico2 < 0.0f) pico2 = 0.0f;

            if (pajaro3Rot < -20.0f) pajaro3step = false;
        }
    }
    else {
        // Vuelve a neutro
        pajaro3head = 0.0f;
        pajaro3Rot = 0.0f;
        pajaro3tail = 0.0f;
        pico1 = 0.0f;
        pico2 = 0.0f;
        pajaro3step = false;
    }

    // Hábitat 4 (Oso, Pingüino, Foca)
    if (animacionH4) {
        pBearWalkTime += deltaTime * 4.0f;

        float swingA = std::sin(pBearWalkTime) * 20.0f;
        float swingB = std::sin(pBearWalkTime + glm::pi<float>()) * 20.0f;

        pBearFR_A = swingA;
        pBearBL_A = swingA;
        pBearFL_A = swingB;
        pBearBR_A = swingB;

        if (!pBearTurning) {
            float dx = pBearStep * pBearDir;
            pBearPos.x += dx;
            pBearMoved += fabsf(dx);
            if (pBearMoved >= pBearSegLen) {
                pBearMoved = 0.0f;
                pBearTurning = true;
                pBearTurnLeft = 180.0f;
            }
        }
        else {
            float dYaw = std::min(pBearTurnSpeed, pBearTurnLeft);
            pBearRot += dYaw;
            pBearTurnLeft -= dYaw;
            if (pBearTurnLeft <= 0.0f) {
                pBearTurning = false;
                pBearDir *= -1;
            }
            if (pBearRot >= 360.0f) pBearRot -= 360.0f;
            if (pBearRot < 0.0f) pBearRot += 360.0f;
        }

        pinguTime += deltaTime * 3.0f;  // velocidad de animación
        pinguWings_A = std::sin(pinguTime) * 35.0f;
        pinguHead_A = std::sin(pinguTime + glm::pi<float>()) * 15.0f;

        if (!pinguTurning) {
            float dx = pinguStep * pinguDir;
            pinguPos.x += dx;
            pinguMoved += fabsf(dx);
            if (pinguMoved >= pinguSegLen) {
                pinguMoved = 0.0f;
                pinguTurning = true;
                pinguTurnLeft = 180.0f;
            }
        }
        else {
            float dYaw = std::min(pinguTurnSpeed, pinguTurnLeft);
            pinguRot += dYaw;
            pinguTurnLeft -= dYaw;
            if (pinguTurnLeft <= 0.0f) {
                pinguTurning = false;
                pinguDir *= -1;
            }
            if (pinguRot >= 360.0f) pinguRot -= 360.0f;
            if (pinguRot < 0.0f) pinguRot += 360.0f;
        }
        sealTime += deltaTime * 2.5f;

        sealHands_A = std::sin(sealTime) * 30.0f;
        sealHead_A = std::sin(sealTime + glm::pi<float>()) * 15.0f;

        if (!sealTurning) {
            float dx = sealStep * sealDir;
            sealPos.x += dx;
            sealMoved += fabsf(dx);
            if (sealMoved >= sealSegLen) {
                sealMoved = 0.0f;
                sealTurning = true;
                sealTurnLeft = 180.0f;
            }
        }
        else {
            float dYaw = std::min(sealTurnSpeed, sealTurnLeft);
            sealRot += dYaw;
            sealTurnLeft -= dYaw;
            if (sealTurnLeft <= 0.0f) {
                sealTurning = false;
                sealDir *= -1;
            }
            if (sealRot >= 360.0f) sealRot -= 360.0f;
            if (sealRot < 0.0f) sealRot += 360.0f;
        }
    }
    else {
        // Reset de Hábitat 4
        pBearWalkTime = 0.0f;
        pBearFR_A = pBearFL_A = pBearBR_A = pBearBL_A = 0.0f;
        pinguTime = 0.0f;
        pinguHead_A = pinguWings_A = 0.0f;
        sealTime = 0.0f;
        sealHead_A = sealHands_A = 0.0f;
    }

    // ===== Hábitat 1 (Tiburón y Piraña)
    if (animacionH1)
    {
        float dt = deltaTime;
        // ---------- TIBURÓN ----------
        sharkTime += dt;

        // Animación de cabeza, cuerpo y cola
        sharkHeadAngle = 12.0f * std::sin(sharkTime * 2.0f);
        sharkBodyAngle = 10.0f * std::sin(sharkTime * 1.5f);
        sharkTailA = 30.0f * std::sin(sharkTime * 2.5f);


        float zSpeed = 1.5f;
        float xSpeed = 1.0f;
        float zMax = 2.0f;
        float xMax = 3.0f;

        // Avance en Z
        sharkPos.z += sharkZDir * zSpeed * dt;
        if (sharkPos.z > zMax) {
            sharkPos.z = zMax;
            sharkZDir = -1.0f;    // ahora va hacia atrás
        }
        else if (sharkPos.z < -zMax) {
            sharkPos.z = -zMax;
            sharkZDir = 1.0f;     // ahora va hacia adelante
        }

        sharkPos.x += sharkXDir * xSpeed * dt;
        if (sharkPos.x > xMax) {
            sharkPos.x = xMax;
            sharkXDir = -1.0f;
        }
        else if (sharkPos.x < -xMax) {
            sharkPos.x = -xMax;
            sharkXDir = 1.0f;
        }

        // ---------- PIRAÑA ----------
        piraTime += dt;

        float swimSpeed = 0.5f;
        float swimLimit = 0.3f;

        piranhaPos.z += swimSpeed * dt;

        if (piranhaPos.z > swimLimit) {
            piranhaPos.z = swimLimit;
            swimSpeed = -swimSpeed;
        }
        else if (piranhaPos.z < -swimLimit) {
            piranhaPos.z = -swimLimit;
            swimSpeed = -swimSpeed;
        }

        // Movimiento cabeza/cola
        piraHeadAngle = 20.0f * sin(piraTime * 2.0f);
        piraTailAngle = -25.0f * sin(piraTime * 2.8f);
        const float rotMax = 15.0f;
        const float rotSpeed = 25.0f;

        piraRotAngle += rotSpeed * piraDirRot * dt;

        if (piraRotAngle > rotMax) {
            piraRotAngle = rotMax;
            piraDirRot = -1.0f;         // girar hacia el otro lado
        }
        else if (piraRotAngle < -rotMax) {
            piraRotAngle = -rotMax;
            piraDirRot = 1.0f;
        }

        piranhaRot = piraRotAngle;
    }

    // --- Animación de caminata del personaje ---
    if (isWalking) {
        // avanzamos la fase del ciclo
        walkTime += deltaTime * walkSpeed;

        float s = sin(walkTime);   // [-1, 1]

        // brazos van en fase contraria a las piernas
        armRightAngle = maxArmSwingDeg * s;
        armLeftAngle = -maxArmSwingDeg * s;

        legRightAngle = -maxLegSwingDeg * s;
        legLeftAngle = maxLegSwingDeg * s;
    }
    else {
        // si no camina, relajamos poco a poco los ángulos hacia 0
        armRightAngle *= 0.85f;
        armLeftAngle *= 0.85f;
        legRightAngle *= 0.85f;
        legLeftAngle *= 0.85f;
    }
}

// ============================================================================
// CrearObjeto
// Descripción:
//   Crea y configura un VAO con VBO + EBO para un objeto con índice,
//   asumiendo el siguiente layout de vértice:
//
//      [ pos.x, pos.y, pos.z,  tex.u, tex.v,  normal.x, normal.y, normal.z ]
//        (3 floats)           (2)           (3)  -> total: 8 floats por vértice
//
//   Parámetros:
//     - VAO, VBO, EBO : referencias donde se guardan los identificadores de OpenGL.
//     - vertices      : arreglo con los datos de vértices.
//     - indices       : arreglo con los índices de dibujo.
//     - tamV, tamI    : tamaño en bytes de 'vertices' y 'indices'.
// ============================================================================
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

// ============================================================================
// CrearObjetoSkyBox
// Descripción:
//   Crea y configura un VAO + VBO para el cubo del skybox.
//   El layout de vértice es únicamente posición (3 floats) sin texturas
//   ni normales, ya que el muestreo se hace por coordenada de dirección.
//
//   Parámetros:
//     - VAO, VBO : referencias para los identificadores OpenGL.
//     - vertices : arreglo con las posiciones del cubo (-1 a 1).
//     - tamV     : tamaño en bytes del arreglo de vértices.
// ============================================================================
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

// ============================================================================
// DibujarBarda
// Descripción:
//   Dibuja un segmento de barda basado en un cubo unitario texturizado,
//   aplicando transformaciones de posición, rotación y escala en X
//   para ajustar el largo deseado.
//
//   Parámetros:
//     - posicion  : centro del segmento en el mundo.
//     - rotacionY : rotación en grados alrededor del eje Y.
//     - largo     : factor de escala en X para extender la barda.
//     - VAO       : VAO del cubo de barda.
//     - textura   : textura 2D a utilizar.
//     - modelLoc  : ubicación del uniform 'model' en el shader de iluminación.
// ============================================================================
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