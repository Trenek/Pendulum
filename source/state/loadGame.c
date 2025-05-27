#include <cglm.h>

#include "engineCore.h"
#include "state.h"

#include "asset.h"
#include "instanceBuffer.h"

#include "modelBuilder.h"
#include "entity.h"

struct node {
    double mass;
    double length;
    double acc;
};

struct p {
    int pendulumCount;
    int nodeCount;

    struct node (*node)[];
    double (*y)[][2];
};

void freeP(void *pPtr) {
    struct p *p = pPtr;

    free(p->node);
    free(p->y);
    free(p);
}

static void loadInput(struct EngineCore *this) {
    FILE *file = fopen("examples/input.txt", "r");

    struct p *p = malloc(sizeof(struct p));

    fscanf(file, "%d %d", &p->pendulumCount, &p->nodeCount);
    p->node = malloc(sizeof(struct node) * p->pendulumCount * p->nodeCount);
    p->y = malloc(sizeof(double) * p->pendulumCount * p->nodeCount * 2);
    p->node = malloc(sizeof(struct node) * p->pendulumCount * p->nodeCount);
    p->y = malloc(sizeof(double) * p->pendulumCount * p->nodeCount * 2);

    struct node (*node)[p->nodeCount] = p->node;
    double (*y)[p->nodeCount][2] = p->y;

    for (int i = 0; i < p->pendulumCount; i += 1) {
        for (int j = 0; j < p->nodeCount; j += 1) {
            fscanf(file, "%lf %lf %lf %lf", 
                &y[i][j][0], 
                &y[i][j][1], 
                &node[i][j].mass, 
                &node[i][j].length
            );
            node[i][j].acc = 0;
        }
    }

    fclose(file);

    addResource(&this->resource, "Pendulum", p, freeP);
}

static void addEntities(struct EngineCore *this) {
    struct ResourceManager *entityData = calloc(1, sizeof(struct ResourceManager));
    struct ResourceManager *modelData = findResource(&this->resource, "modelData");
    struct p *p = findResource(&this->resource, "Pendulum");

    struct descriptorSetLayout *objectLayout = findResource(findResource(&this->resource, "objectLayout"), "object");

    addResource(entityData, "Node", createModel((struct ModelBuilder) {
        .instanceCount = (p->nodeCount + 1) * p->pendulumCount,
        .modelData = findResource(modelData, "sphere"),
        .objectLayout = objectLayout->descriptorSetLayout,

        INS(instance, instanceBuffer),
    }, &this->graphics), destroyEntity);
    addResource(entityData, "Line", createModel((struct ModelBuilder) {
        .instanceCount = p->nodeCount * p->pendulumCount,
        .modelData = findResource(modelData, "line"),
        .objectLayout = objectLayout->descriptorSetLayout,

        INS(instance, instanceBuffer),
    }, &this->graphics), destroyEntity);

    addResource(&this->resource, "Entity", entityData, cleanupResources);
}

void loadGame(struct EngineCore *engine, enum state *state) {
    loadInput(engine);
    addEntities(engine);

    *state = GAME;
}
