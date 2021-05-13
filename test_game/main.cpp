#include <engine.h>
#include "game.h"
#include "game.cpp"

void main() {
    Window window = {};
    Renderer renderer = {};
    Time time = {};
    Game game = {};
    window.pName = "Marmot";
    window.height = 600;
    window.width = 800;
    startEngine(&window, &renderer, &game.camera);
    initGame(&game, &renderer, &time);
    while (!windowShouldClose(&window)) {
        enginePreUpdate(&window, &time);
        updateGame(&game, &renderer, &time);
        enginePostUpdate(&window, &time);
    }
    stopRenderer();
    stopWindow(&window);
}

