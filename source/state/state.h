#ifndef STATE_H
#define STATE_H

#include <stdint.h>

enum state {
    LOAD_RESOURCES,
    LOAD_SIMULATION,
    MAIN_MENU,
    SIMULATION,
    PAUSE,
    WIN_SCREEN,
    LOSE,
    EXIT
};

struct EngineCore;
struct renderPassObj;

void simulation(struct EngineCore *engine, enum state *state);
void loadResources(struct EngineCore *engine, enum state *state);
void loadSimulation(struct EngineCore *engine, enum state *state);

#endif
