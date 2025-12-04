# Proyecto Final – Zoológico Interactivo 3D en OpenGL
**Computación Gráfica e Interacción Humano Computadora – 2026-1**  
**Tecnologías:** C++, OpenGL 3.3, GLFW, GLEW, Assimp, stb_image, GLM

##  Descripción General
Este proyecto implementa un entorno tridimensional interactivo que simula un zoológico completo con iluminación dinámica, ciclo día/noche, skybox doble, modelos 3D animados y un personaje principal navegable en primera y tercera persona. El sistema está construido desde cero utilizando el pipeline moderno de OpenGL e integra modelos FBX/OBJ mediante Assimp.

##  Características Principales
###  Skybox con transición día/noche
- Mezcla automática entre cubemaps diurno y nocturno.
- Independiente de la posición de la cámara.
- Transiciones suaves sin artefactos.

###  Iluminación Realista
- Modelo Phong con luz direccional, puntual y spotlight.
- Atenuación física.
- Materiales con mapas difusos y especulares.

###  Avatar Animado (Po de Kung Fu Panda)
- Personaje multiparte (torso y extremidades).
- Movimiento de piernas basado en funciones armónicas.
- Control en primera y tercera persona.

###  Fauna Animada
- Pájaros, ciervos, oso y tiburón con trayectorias individuales.

###  Entorno Completo
- Casa, árboles y objetos creados manualmente en OpenGL.
- Modelos 3D importados mediante Assimp.

##  Tecnologías Utilizadas
- OpenGL 3.3 Core
- GLFW
- GLEW
- GLM
- Assimp
- stb_image
- Shaders GLSL

## 📁 Estructura del Proyecto
```
Proyecto/
├── Main.cpp
├── Camera.h
├── Mesh.h
├── Model.h
├── ObjetosOpenGL.h
├── Shader.h
├── Texture.h
├── Window.h
├── Shaders/
│   ├── Lighting.vert
│   ├── Lighting.frag
│   ├── Skybox.vert
│   └── Skybox.frag
├── Models/
└── Textures/
```

##  Ejecución
1. Compilar en Visual Studio, GCC, MinGW o CLion.  
2. Vincular: opengl32, glfw3, glew32, assimp.  
3. Mantener la estructura de carpetas.  
4. Ejecutar.

##  Controles

| Acción                                      | Tecla            |
|---------------------------------------------|------------------|
| Mover cámara adelante (modo libre)          |         W        |
| Mover cámara atrás (modo libre)             |         S        |
| Mover cámara a la izquierda (modo libre)    |         A        |
| Mover cámara a la derecha (modo libre)      |         D        |
| Subir cámara (modo libre)                   |         Z        |
| Bajar cámara (modo libre)                   |         X        |
| Mover avatar adelante                       | ↑ (flecha arriba)|
| Mover avatar atrás                          | ↓ (flecha abajo) |
| Girar avatar a la izquierda                 | ← (flecha izq.)  |
| Girar avatar a la derecha                   | → (flecha der.)  |
| Alternar primera / tercera persona          |         Q        |
| Alternar seguir avatar / cámara libre       |         E        |
| Abrir / cerrar puerta                       |         P        |
| Activar/desactivar hábitat 1 (acuático)     |         U        |
| Activar/desactivar ardillas                 |         R        |
| Activar/desactivar hábitat 2 (ciervos)      |         B        |
| Activar/desactivar pájaros (aviario)        |         C        |
| Activar/desactivar hábitat 4 (oso/pingüino/foca) |     Y       |
| Encender/apagar luces puntuales             |         L        |
| Encender/apagar luz ambiental               |         O        |
| Control de cámara (rotación)                |       Mouse      |
| Salir                                       |        ESC       |

##  Validaciones
- Iluminación Phong funcional.
- Modelos correctamente cargados.
- Texturas y UV correctos.
- Animación jerárquica estable.
- Ciclo día/noche sin artefactos.
- Fauna con trayectorias independientes.

##  Referencias (APA)
Incluye bibliografía técnica:  
- OpenGL Programming Guide  
- OpenGL Superbible  
- Real-Time Rendering  
- Fundamentals of Computer Graphics  
- Physically Based Rendering  
- Documentación Blender, GIMP, Assimp, OBJ  
- Asistencia IA OpenAI (2024)

## 👥 Autores
- Becerra Lara Alison 
- Mendoza González Mario
- Ramirez Cervantes Cesar Romualdo
