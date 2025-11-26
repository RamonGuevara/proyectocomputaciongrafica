#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <cmath>

#include "Shader.h"
#include "Texture.h"   // Para TextureLoading::LoadCubemap

// -----------------------------------------------------------------------------
//  Soporta transición día/noche automáticamente.
// -----------------------------------------------------------------------------
class Skybox
{
public:

    // Constructor:
    //   - shader: Shader del Skybox
    //   - facesDay: 6 texturas para Skybox de día
    //   - facesNight: 6 texturas para Skybox de noche
    //   - cycleDurationSeconds: duración del ciclo día/noche
    Skybox(Shader& shader,
        const std::vector<const GLchar*>& facesDay,
        const std::vector<const GLchar*>& facesNight,
        float cycleDurationSeconds = 60.0f)
        : shader(shader),
        vao(0),
        vbo(0),
        cubemapDay(0),
        cubemapNight(0),
        cycleDuration(cycleDurationSeconds),
        dayFactor(1.0f)
    {
        initCube();
        loadCubemaps(facesDay, facesNight);

        shader.Use();
        viewLoc = glGetUniformLocation(shader.Program, "view");
        projLoc = glGetUniformLocation(shader.Program, "projection");
        blendLoc = glGetUniformLocation(shader.Program, "blendFactor");

        daySamplerLoc = glGetUniformLocation(shader.Program, "skyboxDay");
        nightSamplerLoc = glGetUniformLocation(shader.Program, "skyboxNight");

        // Día -> unidad 1, noche -> unidad 2
        if (daySamplerLoc >= 0) glUniform1i(daySamplerLoc, 1);
        if (nightSamplerLoc >= 0) glUniform1i(nightSamplerLoc, 2);
    }

    // Destructor (limpia VAO/VBO y texturas)
    ~Skybox()
    {
        if (vbo) glDeleteBuffers(1, &vbo);
        if (vao) glDeleteVertexArrays(1, &vao);
        if (cubemapDay)   glDeleteTextures(1, &cubemapDay);
        if (cubemapNight) glDeleteTextures(1, &cubemapNight);
    }

    // Actualiza ciclo día/noche según tiempo global
    void Update(float globalTime)
    {
        if (cycleDuration <= 0.0f)
        {
            dayFactor = 1.0f;
            return;
        }

        float phase = fmod(globalTime, cycleDuration) / cycleDuration; // [0,1)
        // Seno suavizado: 0 noche -> 1 día
        dayFactor = 0.5f + 0.5f * sin(2.0f * 3.14159265f * phase);
    }

    // Renderiza el skybox mezclando día/noche
    void Render(const glm::mat4& view, const glm::mat4& projection)
    {
        glDepthFunc(GL_LEQUAL);
        glDepthMask(GL_FALSE);

        shader.Use();

        // Vista sin traslación
        glm::mat4 skyView = glm::mat4(glm::mat3(view));

        if (viewLoc >= 0) glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(skyView));
        if (projLoc >= 0) glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        if (blendLoc >= 0)
        {
            float nightFactor = 1.0f - dayFactor;
            glUniform1f(blendLoc, nightFactor);
        }

        glBindVertexArray(vao);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapDay);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapNight);

        glDrawArrays(GL_TRIANGLES, 0, 36);

        glBindVertexArray(0);

        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LESS);
    }

    float GetDayFactor() const { return dayFactor; }

private:

    // --- Datos privados del skybox ---
    Shader& shader;

    GLuint vao, vbo;
    GLuint cubemapDay;
    GLuint cubemapNight;

    GLint viewLoc, projLoc, blendLoc;
    GLint daySamplerLoc, nightSamplerLoc;

    float cycleDuration;
    float dayFactor;

    // -------------------------------------------------------------------------
    // Inicialización del cubo (VAO / VBO)
    // -------------------------------------------------------------------------
    void initCube()
    {
        static const GLfloat SKYBOX_VERTICES[] =
        {
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
             1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

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

        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);

        glBindVertexArray(vao);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER,
            sizeof(SKYBOX_VERTICES),
            SKYBOX_VERTICES,
            GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
            3 * sizeof(GLfloat), (GLvoid*)0);
        glEnableVertexAttribArray(0);

        glBindVertexArray(0);
    }

    // -------------------------------------------------------------------------
    // Cargar cubemaps de día y noche
    // -------------------------------------------------------------------------
    void loadCubemaps(const std::vector<const GLchar*>& facesDay,
        const std::vector<const GLchar*>& facesNight)
    {
        cubemapDay = TextureLoading::LoadCubemap(facesDay);
        cubemapNight = TextureLoading::LoadCubemap(facesNight);
    }
};
