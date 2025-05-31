#include <cglm.h>

#include "renderPassObj.h"
#include "engineCore.h"
#include "state.h"

#include "asset.h"
#include "instanceBuffer.h"

#include "modelBuilder.h"
#include "entity.h"

#include "pendulum.h"

static void loadInput(struct EngineCore *this) {
    FILE *file = fopen("examples/input.txt", "r");

    struct system *p = malloc(sizeof(struct system));

    fscanf(file, "%d %d", &p->pendulumCount, &p->nodeCount);

    struct node (*node)[p->pendulumCount][p->nodeCount] = (void *)(p->node = malloc(sizeof(struct node) * p->pendulumCount * p->nodeCount * 4));

    for (int i = 0; i < p->pendulumCount; i += 1) {
        for (int j = 0; j < p->nodeCount; j += 1) {
            fscanf(file, "%lf %lf %lf %lf", 
                &node[0][i][j].angle,
                &node[0][i][j].angularVelocity,
                &node[0][i][j].mass, 
                &node[0][i][j].length
            );
            node[0][i][j].angle = glm_rad(node[0][i][j].angle);
            for (int k = 1; k < 4; k += 1) {
                node[k][i][j] = node[0][i][j];
            }
        }
    }

    fclose(file);

    addResource(&this->resource, "Pendulum", p, freeSystem);
}

static void addEntities(struct EngineCore *this) {
    struct ResourceManager *entityData = calloc(1, sizeof(struct ResourceManager));
    struct ResourceManager *modelData = findResource(&this->resource, "modelData");
    struct system *p = findResource(&this->resource, "Pendulum");

    struct descriptorSetLayout *objectLayout = findResource(findResource(&this->resource, "objectLayout"), "object");

    addResource(entityData, "Node1", createModel((struct ModelBuilder) {
        .instanceCount = (p->nodeCount + 1) * p->pendulumCount,
        .modelData = findResource(modelData, "sphere"),
        .objectLayout = objectLayout->descriptorSetLayout,

        INS(instance, instanceBuffer),
    }, &this->graphics), destroyEntity);
    addResource(entityData, "Line1", createModel((struct ModelBuilder) {
        .instanceCount = p->nodeCount * p->pendulumCount,
        .modelData = findResource(modelData, "line"),
        .objectLayout = objectLayout->descriptorSetLayout,

        INS(instance, instanceBuffer),
    }, &this->graphics), destroyEntity);
    addResource(entityData, "Node2", createModel((struct ModelBuilder) {
        .instanceCount = (p->nodeCount + 1) * p->pendulumCount,
        .modelData = findResource(modelData, "sphere"),
        .objectLayout = objectLayout->descriptorSetLayout,

        INS(instance, instanceBuffer),
    }, &this->graphics), destroyEntity);
    addResource(entityData, "Line2", createModel((struct ModelBuilder) {
        .instanceCount = p->nodeCount * p->pendulumCount,
        .modelData = findResource(modelData, "line"),
        .objectLayout = objectLayout->descriptorSetLayout,

        INS(instance, instanceBuffer),
    }, &this->graphics), destroyEntity);
    addResource(entityData, "Node3", createModel((struct ModelBuilder) {
        .instanceCount = (p->nodeCount + 1) * p->pendulumCount,
        .modelData = findResource(modelData, "sphere"),
        .objectLayout = objectLayout->descriptorSetLayout,

        INS(instance, instanceBuffer),
    }, &this->graphics), destroyEntity);
    addResource(entityData, "Line3", createModel((struct ModelBuilder) {
        .instanceCount = p->nodeCount * p->pendulumCount,
        .modelData = findResource(modelData, "line"),
        .objectLayout = objectLayout->descriptorSetLayout,

        INS(instance, instanceBuffer),
    }, &this->graphics), destroyEntity);
    addResource(entityData, "Node4", createModel((struct ModelBuilder) {
        .instanceCount = (p->nodeCount + 1) * p->pendulumCount,
        .modelData = findResource(modelData, "sphere"),
        .objectLayout = objectLayout->descriptorSetLayout,

        INS(instance, instanceBuffer),
    }, &this->graphics), destroyEntity);
    addResource(entityData, "Line4", createModel((struct ModelBuilder) {
        .instanceCount = p->nodeCount * p->pendulumCount,
        .modelData = findResource(modelData, "line"),
        .objectLayout = objectLayout->descriptorSetLayout,

        INS(instance, instanceBuffer),
    }, &this->graphics), destroyEntity);

    addResource(&this->resource, "Entity", entityData, cleanupResources);
}

void loadScreens(struct EngineCore *this) {
    struct ResourceManager *screenData = calloc(1, sizeof(struct ResourceManager));

    struct ResourceManager *graphicsPipelineData = findResource(&this->resource, "graphicPipelines");
    struct ResourceManager *entityData = findResource(&this->resource, "Entity");

    struct graphicsPipeline *pipe[] = {
        findResource(graphicsPipelineData, "Floor"),
    };

    struct Entity *entity[] = {
        findResource(entityData, "Node1"),
        findResource(entityData, "Line1"),
        findResource(entityData, "Node2"),
        findResource(entityData, "Line2"),
        findResource(entityData, "Node3"),
        findResource(entityData, "Line3"),
        findResource(entityData, "Node4"),
        findResource(entityData, "Line4"),
    };

    struct ResourceManager *renderPassCoreData = findResource(&this->resource, "RenderPassCoreData");
    struct renderPassCore *renderPassArr[] = { 
        findResource(renderPassCoreData, "Clean"),
        findResource(renderPassCoreData, "Stay")
    };

    addResource(screenData, "One", createRenderPassObj((struct renderPassBuilder){
        .renderPass = renderPassArr[0],
        .coordinates = { 0.0, 0.0, 0.5, 0.5 },
        .data = (struct pipelineConnection[]) {
            {
                .pipe = pipe[0],
                .entity = (struct Entity* []) {
                    entity[0],
                    entity[1],
                },
                .qEntity = 2
            },
        },
        .qData = 1,
        .updateCameraBuffer = updateFirstPersonCameraBuffer,
    }, &this->graphics), destroyRenderPassObj);
    addResource(screenData, "Two", createRenderPassObj((struct renderPassBuilder){
        .renderPass = renderPassArr[0],
        .coordinates = { 0.5, 0.0, 0.5, 0.5 },
        .data = (struct pipelineConnection[]) {
            {
                .pipe = pipe[0],
                .entity = (struct Entity* []) {
                    entity[2],
                    entity[3],
                },
                .qEntity = 2
            },
        },
        .qData = 1,
        .updateCameraBuffer = updateFirstPersonCameraBuffer
    }, &this->graphics), destroyRenderPassObj);
    addResource(screenData, "Three", createRenderPassObj((struct renderPassBuilder){
        .renderPass = renderPassArr[0],
        .coordinates = { 0.0, 0.5, 0.5, 0.5 },
        .data = (struct pipelineConnection[]) {
            {
                .pipe = pipe[0],
                .entity = (struct Entity* []) {
                    entity[4],
                    entity[5],
                },
                .qEntity = 2
            },
        },
        .qData = 1,
        .updateCameraBuffer = updateFirstPersonCameraBuffer
    }, &this->graphics), destroyRenderPassObj);
    addResource(screenData, "Four", createRenderPassObj((struct renderPassBuilder){
        .renderPass = renderPassArr[0],
        .coordinates = { 0.5, 0.5, 0.5, 0.5 },
        .data = (struct pipelineConnection[]) {
            {
                .pipe = pipe[0],
                .entity = (struct Entity* []) {
                    entity[6],
                    entity[7],
                },
                .qEntity = 2
            },
        },
        .qData = 1,
        .updateCameraBuffer = updateFirstPersonCameraBuffer
    }, &this->graphics), destroyRenderPassObj);

    ((struct renderPassObj *)findResource(screenData, "One"))->camera = initCamera();
    ((struct renderPassObj *)findResource(screenData, "Two") )->camera = initCamera();
    ((struct renderPassObj *)findResource(screenData, "Three"))->camera = initCamera();
    ((struct renderPassObj *)findResource(screenData, "Four"))->camera = initCamera();

    addResource(&this->resource, "ScreenData", screenData, cleanupResources);
}

void loadSimulation(struct EngineCore *engine, enum state *state) {
    loadInput(engine);
    addEntities(engine);

    loadScreens(engine);

    *state = SIMULATION;
}
