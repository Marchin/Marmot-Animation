#ifndef GAME_H
#define GAME_H

struct Game {
    Camera camera;
    Transform empty;
    Shader modelShader;
    ModelStructure modelStructure;
    f32 camX, camY;
};

#endif //GAME_H