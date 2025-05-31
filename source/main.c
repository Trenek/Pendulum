#include "engineCore.h"

#include "state.h"

int main() {
    struct EngineCore engine = setup();
    void (* const state[])(struct EngineCore *engine, enum state *state) = {
        [SIMULATION] = simulation,
        [LOAD_SIMULATION] = loadSimulation,
        [LOAD_RESOURCES] = loadResources,
    };
    enum state stateID = LOAD_RESOURCES;

    do {
        state[stateID](&engine, &stateID);
    } while (stateID != EXIT && !shouldWindowClose(engine.window));

    cleanup(engine);

    return 0;
}
