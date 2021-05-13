#include "engine.h"

//#include "kiss_fft/kiss_fft.c"

#include "rotor.cpp"
#if OPENGL
#include "opengl_renderer.cpp"
#endif

#include "components.cpp"
#include "camera.cpp"
#include "models_parser.cpp"

ENGINE_API void
startEngine(Window* pWindow, Renderer* pRenderer, Camera* pCamera) {
    gpMeshComponentsPool = (MeshComponentsPool*)malloc(sizeof(MeshComponentsPool));
    assert(gpMeshComponentsPool);
    memset(gpMeshComponentsPool, 0, sizeof(MeshComponentsPool));
    
    startWindow(pWindow);
    startRenderer(pRenderer, pWindow, pCamera);
}

ENGINE_API void
enginePreUpdate(Window* pWindow, Time* pTime) {
    clearRenderer();
    f32 currentFrame = getTime();
    pTime->deltaTime = currentFrame - pTime->lastFrame;
    pTime->lastFrame = currentFrame;
}

ENGINE_API void
enginePostUpdate(Window* pWindow, Time* pTime) {
    swapBuffers(pWindow);
    pollEventsFromWindow(pWindow);
}