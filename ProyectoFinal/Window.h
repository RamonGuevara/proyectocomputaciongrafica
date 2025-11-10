#pragma once

#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

class Window
{
public:
    Window(GLuint width = 800, GLuint height = 600, const char* title = "Proyecto")
        : width(width), height(height),
        window(nullptr),
        lastX(width / 2.0f), lastY(height / 2.0f),
        firstMouse(true),
        xChange(0.0f), yChange(0.0f),
        deltaTime(0.0f), lastFrame(0.0f)
    {
        // Inicializar GLFW
        if (!glfwInit())
        {
            std::cerr << "ERROR::GLFW::INIT_FAILED" << std::endl;
            return;
        }

        // Crear ventana
        window = glfwCreateWindow(width, height, title, nullptr, nullptr);
        if (!window)
        {
            std::cerr << "ERROR::GLFW::WINDOW_CREATION_FAILED" << std::endl;
            glfwTerminate();
            return;
        }

        glfwMakeContextCurrent(window);

        // Tamaño del framebuffer real
        glfwGetFramebufferSize(window, &bufferWidth, &bufferHeight);

        // Asignar este objeto como "user pointer" para los callbacks estáticos
        glfwSetWindowUserPointer(window, this);

        // Callbacks
        glfwSetKeyCallback(window, HandleKeys);
        glfwSetCursorPosCallback(window, HandleMouse);

        // Capturar el mouse (estilo cámara FPS)
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        // Inicializar GLEW
        glewExperimental = GL_TRUE;
        if (glewInit() != GLEW_OK)
        {
            std::cerr << "ERROR::GLEW::INIT_FAILED" << std::endl;
            glfwDestroyWindow(window);
            glfwTerminate();
            window = nullptr;
            return;
        }

        // Viewport inicial
        glViewport(0, 0, bufferWidth, bufferHeight);

        // Depth test global
        glEnable(GL_DEPTH_TEST);

        // Inicializar arreglo de teclas
        for (int i = 0; i < 1024; i++)
            keys[i] = false;
    }

    ~Window()
    {
        if (window)
        {
            glfwDestroyWindow(window);
            glfwTerminate();
        }
    }

    // ¿La ventana debe cerrarse?
    bool ShouldClose() const
    {
        return window == nullptr || glfwWindowShouldClose(window);
    }

    // Intercambia buffers
    void SwapBuffers()
    {
        if (window)
            glfwSwapBuffers(window);
    }

    // Procesa eventos de GLFW
    void PollEvents()
    {
        glfwPollEvents();
    }

    // Debe llamarse una vez por frame al inicio del loop
    void UpdateDeltaTime()
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
    }

    float GetDeltaTime() const { return deltaTime; }

    // Consulta de teclas
    bool IsKeyPressed(int key) const
    {
        if (key < 0 || key >= 1024) return false;
        return keys[key];
    }

    // Cambios de mouse (offset desde el último frame)
    float GetXChange()
    {
        float temp = xChange;
        xChange = 0.0f;
        return temp;
    }

    float GetYChange()
    {
        float temp = yChange;
        yChange = 0.0f;
        return temp;
    }

    // Accesores útiles
    GLFWwindow* GetGLFWwindow() const { return window; }

    int GetBufferWidth() const { return bufferWidth; }
    int GetBufferHeight() const { return bufferHeight; }

private:
    GLFWwindow* window;

    GLuint width, height;
    int bufferWidth, bufferHeight;

    bool keys[1024];

    float lastX, lastY;
    bool firstMouse;
    float xChange, yChange;

    float deltaTime;
    float lastFrame;

    // --- Callbacks estáticos (GLFW) ---

    static void HandleKeys(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        Window* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if (!self) return;

        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, GL_TRUE);
        }

        if (key >= 0 && key < 1024)
        {
            if (action == GLFW_PRESS)
                self->keys[key] = true;
            else if (action == GLFW_RELEASE)
                self->keys[key] = false;
        }
    }

    static void HandleMouse(GLFWwindow* window, double xPos, double yPos)
    {
        Window* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if (!self) return;

        if (self->firstMouse)
        {
            self->lastX = static_cast<float>(xPos);
            self->lastY = static_cast<float>(yPos);
            self->firstMouse = false;
        }

        float xOffset = static_cast<float>(xPos) - self->lastX;
        float yOffset = self->lastY - static_cast<float>(yPos); // invertido

        self->lastX = static_cast<float>(xPos);
        self->lastY = static_cast<float>(yPos);

        self->xChange += xOffset;
        self->yChange += yOffset;
    }
};

