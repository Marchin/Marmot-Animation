#ifndef ENGINE_H
#define ENGINE_H
#define _CRT_SECURE_NO_DEPRECATE
#include <stdio.h>
#include <float.h>

#include "meow_intrinsics.h"
#include "meow_hash.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define MAX_PATH_SIZE 260

#if OPENGL
#include "glad.c"
#include <glfw3.h>
#define NEAREST GL_NEAREST
#define LINEAR GL_LINEAR
#define CLAMP_TO_EDGE GL_CLAMP_TO_EDGE
#define REPEAT GL_REPEAT
#define MIRRORED_REPEAT GL_MIRRORED_REPEAT
#define SHADER_STORAGE_BUFFER GL_SHADER_STORAGE_BUFFER

#define KEY_W GLFW_KEY_W
#define KEY_S GLFW_KEY_S
#define KEY_A GLFW_KEY_A
#define KEY_D GLFW_KEY_D
#define KEY_Q GLFW_KEY_Q
#define KEY_E GLFW_KEY_E
#define KEY_Z GLFW_KEY_Z
#define KEY_C GLFW_KEY_C
#define KEY_LEFT GLFW_KEY_LEFT
#define KEY_RIGHT GLFW_KEY_RIGHT
#define KEY_UP GLFW_KEY_UP
#define KEY_DOWN GLFW_KEY_DOWN
#define KEY_SPACE GLFW_KEY_SPACE
#endif

#if ENGINE_EXPORTS
#define ENGINE_API __declspec(dllexport)
#else
#define ENGINE_API __declspec(dllimport)
#endif

#include "base.h"

#define HANDMADE_MATH_IMPLEMENTATION
#include "handmade_math.h"

#include "utils.h"
#include "rotor.h"

#if OPENGL
#include "opengl_renderer.h"
#endif

struct Level;
#include "components.h"
#include "camera.h"
#include "models_parser.h"

struct ENGINE_API Time {
    f32 deltaTime;
    f32 lastFrame;
};

global const hmm_vec3 VEC3_X = HMM_Vec3(1.f, 0.f, 0.f);
global const hmm_vec3 VEC3_Y = HMM_Vec3(0.f, 1.f, 0.f);
global const hmm_vec3 VEC3_Z = HMM_Vec3(0.f, 0.f, 1.f);

global MeshComponentsPool* gpMeshComponentsPool;

struct Level {
    Plane* pBSPPlanes;
    u32 bspPlaneCount;
    u32 maxBSPPlanes;
};

#include "collada.cpp"

ENGINE_API void startEngine(Window* pWindow, Renderer* pRenderer, Camera* pCamera);
ENGINE_API void enginePreUpdate(Window* pWindow, Time* pTime);
ENGINE_API void enginePostUpdate(Window* pWindow, Time* pTime);

#endif //ENGINE_H