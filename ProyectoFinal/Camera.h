#pragma once

// Std. Includes
#include <vector>

// GL Includes
#define GLEW_STATIC
#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Tipos de movimiento de cámara
enum Camera_Movement
{
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,      // Nuevo: subir (camlift)
    DOWN     // Nuevo: bajar (camlift)
};

// Valores por defecto
const GLfloat YAW = -90.0f;
const GLfloat PITCH = 0.0f;
const GLfloat SPEED = 6.0f;
const GLfloat SENSITIVTY = 0.25f;
const GLfloat ZOOM = 45.0f;

class Camera
{
public:
    // Constructor con vectores
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
        GLfloat yaw = YAW,
        GLfloat pitch = PITCH)
        : front(glm::vec3(0.0f, 0.0f, -1.0f)),
        movementSpeed(SPEED),
        mouseSensitivity(SENSITIVTY),
        zoom(ZOOM)
    {
        this->position = position;
        this->worldUp = up;
        this->yaw = yaw;
        this->pitch = pitch;
        this->updateCameraVectors();
    }

    // Constructor con escalares
    Camera(GLfloat posX, GLfloat posY, GLfloat posZ,
        GLfloat upX, GLfloat upY, GLfloat upZ,
        GLfloat yaw, GLfloat pitch)
        : front(glm::vec3(0.0f, 0.0f, -1.0f)),
        movementSpeed(SPEED),
        mouseSensitivity(SENSITIVTY),
        zoom(ZOOM)
    {
        this->position = glm::vec3(posX, posY, posZ);
        this->worldUp = glm::vec3(upX, upY, upZ);
        this->yaw = yaw;
        this->pitch = pitch;
        this->updateCameraVectors();
    }

    // Matriz de vista
    glm::mat4 GetViewMatrix() const
    {
        return glm::lookAt(this->position, this->position + this->front, this->up);
    }

    // Movimiento con teclado
    void ProcessKeyboard(Camera_Movement direction, GLfloat deltaTime)
    {
        GLfloat velocity = this->movementSpeed * deltaTime;

        if (direction == FORWARD)
            this->position += this->front * velocity;
        if (direction == BACKWARD)
            this->position -= this->front * velocity;
        if (direction == LEFT)
            this->position -= this->right * velocity;
        if (direction == RIGHT)
            this->position += this->right * velocity;
        if (direction == UP)
            this->position += this->worldUp * velocity;
        if (direction == DOWN)
            this->position -= this->worldUp * velocity;
    }

    // Movimiento con mouse (rotación)
    void ProcessMouseMovement(GLfloat xOffset, GLfloat yOffset, GLboolean constrainPitch = true)
    {
        xOffset *= this->mouseSensitivity;
        yOffset *= this->mouseSensitivity;

        this->yaw += xOffset;
        this->pitch += yOffset;

        if (constrainPitch)
        {
            if (this->pitch > 89.0f)
                this->pitch = 89.0f;
            if (this->pitch < -89.0f)
                this->pitch = -89.0f;
        }

        updateCameraVectors();
    }

    // Zoom con scroll
    void ProcessMouseScroll(GLfloat yOffset)
    {
        this->zoom -= yOffset;

        if (this->zoom < 1.0f)
            this->zoom = 1.0f;
        if (this->zoom > 45.0f)
            this->zoom = 45.0f;
    }

    GLfloat GetZoom() const { return this->zoom; }
    glm::vec3 GetPosition() const { return this->position; }
    glm::vec3 GetFront() const { return this->front; }

private:
    // Atributos de cámara
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 worldUp;

    // Ángulos Euler
    GLfloat yaw;
    GLfloat pitch;

    // Opciones
    GLfloat movementSpeed;
    GLfloat mouseSensitivity;
    GLfloat zoom;

    // Recalcula front/right/up a partir de yaw/pitch
    void updateCameraVectors()
    {
        glm::vec3 frontVec;
        frontVec.x = cos(glm::radians(this->yaw)) * cos(glm::radians(this->pitch));
        frontVec.y = sin(glm::radians(this->pitch));
        frontVec.z = sin(glm::radians(this->yaw)) * cos(glm::radians(this->pitch));
        this->front = glm::normalize(frontVec);

        this->right = glm::normalize(glm::cross(this->front, this->worldUp));
        this->up = glm::normalize(glm::cross(this->right, this->front));
    }
};
