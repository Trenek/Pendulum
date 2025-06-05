#include <cglm.h>
#include <string.h>

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

    p->qMethod = 4;
    p->method = malloc(sizeof(struct method));

    p->method[0] = (struct method) {
        .name = "Euler",
        .coords = { 0.0, 0.0, 0.5, 0.5 },
        .color = { 0.5, 0.5, 0.5, 1.0 },
        .f = euler
    };
    p->method[1] = (struct method) {
        .name = "Mod Euler",
        .coords = { 0.5, 0.0, 0.5, 0.5 },
        .color = { 0.5, 0.5, 0.5, 1.0 },
        .f = modified_euler
    };
    p->method[2] = (struct method) {
        .name = "Heun",
        .coords = { 0.0, 0.5, 0.5, 0.5 },
        .color = { 0.5, 0.5, 0.5, 1.0 },
        .f = heun
    };
    p->method[3] = (struct method) {
        .name = "RK4",
        .coords = { 0.5, 0.5, 0.5, 0.5 },
        .color = { 0.5, 0.5, 0.5, 1.0 },
        .f = heun
    };

    fscanf(file, "%f", &p->time);
    fscanf(file, "%f %f %f", &p->pos[0], &p->pos[1], &p->pos[2]);
    fscanf(file, "%f %f", &p->tilt[0], &p->tilt[1]);

    fscanf(file, "%d %d", &p->pendulumCount, &p->nodeCount);

    struct node (*node)[p->pendulumCount][p->nodeCount] = (void *)(p->node = malloc(sizeof(struct node) * p->pendulumCount * p->nodeCount * p->qMethod));

    for (int i = 0; i < p->pendulumCount; i += 1) {
        for (int j = 0; j < p->nodeCount; j += 1) {
            fscanf(file, "%lf %lf %lf %lf", 
                &node[0][i][j].th,
                &node[0][i][j].dth,
                &node[0][i][j].mass, 
                &node[0][i][j].length
            );
            node[0][i][j].th = glm_rad(node[0][i][j].th);
            for (int k = 1; k < p->qMethod; k += 1) {
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

    char buffer[50];

    for (int i = 0; i < p->qMethod; i += 1) {
        sprintf(buffer, "Node%d", i);
        printf("%s\n", buffer);
        addResource(entityData, buffer, createModel((struct ModelBuilder) {
            .instanceCount = (p->nodeCount + 1) * p->pendulumCount,
            .modelData = findResource(modelData, "sphere"),
            .objectLayout = objectLayout->descriptorSetLayout,

            INS(instance, instanceBuffer),
        }, &this->graphics), destroyEntity);

        sprintf(buffer, "Line%d", i);
        printf("%s\n", buffer);
        addResource(entityData, buffer, createModel((struct ModelBuilder) {
            .instanceCount = p->nodeCount * p->pendulumCount,
            .modelData = findResource(modelData, "line"),
            .objectLayout = objectLayout->descriptorSetLayout,

            INS(instance, instanceBuffer),
        }, &this->graphics), destroyEntity);

        sprintf(buffer, "Name %d", i);
        printf("%s\n", buffer);
        addString(entityData, modelData, objectLayout, this, buffer, p->method[i].name);
    }

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

    struct ResourceManager *renderPassCoreData = findResource(&this->resource, "RenderPassCoreData");
    struct renderPassCore *renderPassArr[] = { 
        findResource(renderPassCoreData, "Clean"),
        findResource(renderPassCoreData, "Stay")
    };

    printf("Here\n");
    char buffer[5][50];
    for (int i = 0; i < p->qMethod; i += 1) {
        sprintf(buffer[0], "Screen %d", i);
        sprintf(buffer[1], "Node%d", i);
        sprintf(buffer[2], "Line%d", i);
        sprintf(buffer[3], "Text Screen %d", i);
        sprintf(buffer[4], "Name %d", i);

        printf("%s\n", buffer[0]);
        addResource(screenData, buffer[0], createRenderPassObj((struct renderPassBuilder){
            .renderPass = renderPassArr[0],
            .coordinates = {
                p->method[i].coords[0],
                p->method[i].coords[1],
                p->method[i].coords[2],
                p->method[i].coords[3],
            },
            .data = (struct pipelineConnection[]) {
                {
                    .pipe = pipe[0],
                    .entity = (struct Entity* []) {
                        findResource(entityData, buffer[1]),
                        findResource(entityData, buffer[2]),
                    },
                    .qEntity = 2
                },
            },
            .qData = 1,
            .updateCameraBuffer = updateFirstPersonCameraBuffer,
        }, &this->graphics), destroyRenderPassObj);
        printf("%s\n", buffer[3]);
        addResource(screenData, buffer[3], createRenderPassObj((struct renderPassBuilder){
            .renderPass = renderPassArr[1],
            .coordinates = {
                p->method[i].coords[0],
                p->method[i].coords[1],
                p->method[i].coords[2],
                p->method[i].coords[3],
            },
            .data = (struct pipelineConnection[]) {
                {
                    .pipe = pipe[1],
                    .entity = (struct Entity* []) {
                        findResource(entityData, buffer[4]),
                    },
                    .qEntity = 1
                }
            },
            .qData = 1,
            .updateCameraBuffer = updateFirstPersonCameraBuffer,
        }, &this->graphics), destroyRenderPassObj);
        struct camera *camera[] = {
            &((struct renderPassObj *)findResource(screenData, buffer[0]))->camera,
            &((struct renderPassObj *)findResource(screenData, buffer[3]))->camera,
        };

        for (size_t j = 0; j < sizeof(camera) / sizeof(struct camera *); j += 1) {
            camera[j]->pos[0] = p->pos[0];
            camera[j]->pos[1] = p->pos[1];
            camera[j]->pos[2] = p->pos[2];
            camera[j]->tilt[0] = p->tilt[0];
            camera[j]->tilt[1] = p->tilt[1];
        };

        struct instance *text = ((struct Entity *)findResource(entityData, buffer[4]))->instance;

        *text = (struct instance){
            .pos = { 0.0f, 0.3f, 0.0f },
            .rotation = { 0.0f, 0.0f, 0.0f },
            .fixedRotation = { 0.0f, 0.0f, 0.0f },
            .scale = { 4 * 10e-3, 4 * 10e-3, 4 * 10e-3 },
            .textureIndex = 0,
            .shadow = false
        };
    }
}

void loadSimulation(struct EngineCore *engine, enum state *state) {
    loadInput(engine);
    addEntities(engine);

    loadScreens(engine);
    printf("Loaded\n");

    *state = SIMULATION;
}
