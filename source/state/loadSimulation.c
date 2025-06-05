#include <cglm.h>

#include "renderPassObj.h"
#include "engineCore.h"
#include "state.h"

#include "asset.h"
#include "instanceBuffer.h"

#include "modelBuilder.h"
#include "stringBuilder.h"
#include "entity.h"

#include "pendulum.h"

static void loadInput(struct EngineCore *this) {
    FILE *file = fopen("examples/input.txt", "r");

    struct system *p = malloc(sizeof(struct system));

    fscanf(file, "%f", &p->time);
    fscanf(file, "%f %f %f", &p->pos[0], &p->pos[1], &p->pos[2]);
    fscanf(file, "%f %f", &p->tilt[0], &p->tilt[1]);

    fscanf(file, "%d %d", &p->pendulumCount, &p->nodeCount);

    struct node (*node)[p->pendulumCount][p->nodeCount] = (void *)(p->node = malloc(sizeof(struct node) * p->pendulumCount * p->nodeCount * 4));

    for (int i = 0; i < p->pendulumCount; i += 1) {
        for (int j = 0; j < p->nodeCount; j += 1) {
            fscanf(file, "%lf %lf %lf %lf", 
                &node[0][i][j].th,
                &node[0][i][j].dth,
                &node[0][i][j].mass, 
                &node[0][i][j].length
            );
            node[0][i][j].th = glm_rad(node[0][i][j].th);
            for (int k = 1; k < 4; k += 1) {
                node[k][i][j] = node[0][i][j];
            }
        }
    }

    fclose(file);

    addResource(&this->resource, "Pendulum", p, freeSystem);
}

void addString(
    struct ResourceManager *entityData,
    struct ResourceManager *modelData,

    struct descriptorSetLayout *objectLayout,
    struct EngineCore *this,
    const char *name,
    const char *buffer
) {
    addResource(entityData, name, createString((struct StringBuilder) {
        .instanceCount = 1,
        .string = buffer,
        .modelData = findResource(modelData, "font"),
        .objectLayout = objectLayout->descriptorSetLayout,

        INS(instance, instanceBuffer),
        .center = 0
    }, &this->graphics), destroyEntity);
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
    addString(entityData, modelData, objectLayout, this, "Name 1", "Euler");
    addString(entityData, modelData, objectLayout, this, "Name 2", "Heun");
    addString(entityData, modelData, objectLayout, this, "Name 3", "RK5");
    addString(entityData, modelData, objectLayout, this, "Name 4", "RK4");

    addResource(&this->resource, "Entity", entityData, cleanupResources);
}

void loadScreens(struct EngineCore *this) {
    struct ResourceManager *screenData = calloc(1, sizeof(struct ResourceManager));

    struct ResourceManager *graphicsPipelineData = findResource(&this->resource, "graphicPipelines");
    struct ResourceManager *entityData = findResource(&this->resource, "Entity");
    struct system *p = findResource(&this->resource, "Pendulum");

    struct graphicsPipeline *pipe[] = {
        findResource(graphicsPipelineData, "Floor"),
        findResource(graphicsPipelineData, "Text"),
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
        findResource(entityData, "Name 1"),
        findResource(entityData, "Name 2"),
        findResource(entityData, "Name 3"),
        findResource(entityData, "Name 4"),
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
    addResource(screenData, "NOne", createRenderPassObj((struct renderPassBuilder){
        .renderPass = renderPassArr[1],
        .coordinates = { 0.0, 0.0, 0.5, 0.5 },
        .data = (struct pipelineConnection[]) {
            {
                .pipe = pipe[1],
                .entity = (struct Entity* []) {
                    entity[8],
                },
                .qEntity = 1
            }
        },
        .qData = 1,
        .updateCameraBuffer = updateFirstPersonCameraBuffer,
    }, &this->graphics), destroyRenderPassObj);
    addResource(screenData, "NTwo", createRenderPassObj((struct renderPassBuilder){
        .renderPass = renderPassArr[1],
        .coordinates = { 0.5, 0.0, 0.5, 0.5 },
        .data = (struct pipelineConnection[]) {
            {
                .pipe = pipe[1],
                .entity = (struct Entity* []) {
                    entity[9],
                },
                .qEntity = 1
            }
        },
        .qData = 1,
        .updateCameraBuffer = updateFirstPersonCameraBuffer,
    }, &this->graphics), destroyRenderPassObj);
    addResource(screenData, "NThree", createRenderPassObj((struct renderPassBuilder){
        .renderPass = renderPassArr[1],
        .coordinates = { 0.0, 0.5, 0.5, 0.5 },
        .data = (struct pipelineConnection[]) {
            {
                .pipe = pipe[1],
                .entity = (struct Entity* []) {
                    entity[10],
                },
                .qEntity = 1
            }
        },
        .qData = 1,
        .updateCameraBuffer = updateFirstPersonCameraBuffer,
    }, &this->graphics), destroyRenderPassObj);
    addResource(screenData, "NFour", createRenderPassObj((struct renderPassBuilder){
        .renderPass = renderPassArr[1],
        .coordinates = { 0.5, 0.5, 0.5, 0.5 },
        .data = (struct pipelineConnection[]) {
            {
                .pipe = pipe[1],
                .entity = (struct Entity* []) {
                    entity[11],
                },
                .qEntity = 1
            }
        },
        .qData = 1,
        .updateCameraBuffer = updateFirstPersonCameraBuffer,
    }, &this->graphics), destroyRenderPassObj);

    struct camera *camera[] = {
        &((struct renderPassObj *)findResource(screenData, "One"))->camera,
        &((struct renderPassObj *)findResource(screenData, "Two") )->camera,
        &((struct renderPassObj *)findResource(screenData, "Three"))->camera,
        &((struct renderPassObj *)findResource(screenData, "Four"))->camera,
        &((struct renderPassObj *)findResource(screenData, "NOne"))->camera,
        &((struct renderPassObj *)findResource(screenData, "NTwo"))->camera,
        &((struct renderPassObj *)findResource(screenData, "NThree"))->camera,
        &((struct renderPassObj *)findResource(screenData, "NFour"))->camera,
    };

    for (size_t i = 0; i < sizeof(camera) / sizeof(struct camera *); i += 1) {
        camera[i]->pos[0] = p->pos[0];
        camera[i]->pos[1] = p->pos[1];
        camera[i]->pos[2] = p->pos[2];
        camera[i]->tilt[0] = p->tilt[0];
        camera[i]->tilt[1] = p->tilt[1];
    };

    addResource(&this->resource, "ScreenData", screenData, cleanupResources);

    struct instance *text[] = {
        entity[8]->instance,
        entity[9]->instance,
        entity[10]->instance,
        entity[11]->instance,
    };

    *text[0] = 
    *text[1] = 
    *text[2] = 
    *text[3] = (struct instance){
        .pos = { 0.0f, 0.3f, 0.0f },
        .rotation = { 0.0f, 0.0f, 0.0f },
        .fixedRotation = { 0.0f, 0.0f, 0.0f },
        .scale = { 4 * 10e-3, 4 * 10e-3, 4 * 10e-3 },
        .textureIndex = 0,
        .shadow = false
    };
}

void loadSimulation(struct EngineCore *engine, enum state *state) {
    loadInput(engine);
    addEntities(engine);

    loadScreens(engine);

    *state = SIMULATION;
}
