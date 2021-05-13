
internal void
initGame(Game* pGame, Renderer* pRenderer, Time* pTime) {
    fillColor(.2f, .2f, .2f);
    
    initTransform(&pGame->empty);
    
    parseCollada(&pGame->modelStructure, "..//resources//bsp.dae");
    
    addChild(&pGame->modelStructure.transform, &pGame->empty);
}

internal void
updateGame(Game* pGame, Renderer* pRenderer, Time* pTime) {
    V3 up = getRotatedVector(VEC3_Y, pRenderer->pCamera->transform.rotor);
    V3 right = getRotatedVector(VEC3_X, pRenderer->pCamera->transform.rotor);
    V3 front = getRotatedVector(VEC3_Z, pRenderer->pCamera->transform.rotor);
    if (isKeyPressed(pRenderer, KEY_D)) {
        moveCamera(&pGame->camera, 10.f * right, pTime->deltaTime);
    }
    if (isKeyPressed(pRenderer, KEY_A)) {
        moveCamera(&pGame->camera, -10.f * right, pTime->deltaTime);
    }
    if (isKeyPressed(pRenderer, KEY_W)) {
        moveCamera(&pGame->camera, -10.f * front, pTime->deltaTime);
    }
    if (isKeyPressed(pRenderer, KEY_S)) {
        moveCamera(&pGame->camera, 10.f * front, pTime->deltaTime);
    }
    if (isKeyPressed(pRenderer, KEY_E)) {
        //moveCamera(&pGame->camera, up, pTime->deltaTime);
    }
    if (isKeyPressed(pRenderer, KEY_Q)) {
        //moveCamera(&pGame->camera, -1.f * up, pTime->deltaTime);
    }
    if (isKeyPressed(pRenderer, KEY_Z)) {
        pRenderer->pCamera->roll -= pTime->deltaTime * 5.f;
        pRenderer->pCamera->roll -= pTime->deltaTime * 5.f;
        updateCameraVectors(pRenderer->pCamera);
    }
    if (isKeyPressed(pRenderer, KEY_C)) {
        pRenderer->pCamera->roll += pTime->deltaTime * 5.f;
        updateCameraVectors(pRenderer->pCamera);
    }
    
    const hmm_mat4 diag = HMM_Mat4d(1.f);
    transformUpdate(&pGame->empty, pTime->deltaTime, diag);
    transformDraw(&pGame->empty, pRenderer, diag);
    
    f64 x, y;
    getMousePos(pRenderer->pWindow, &x, &y);
    cameraMouseMovement(pRenderer->pCamera, x, y, true);
}
