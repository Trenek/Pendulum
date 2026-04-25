#include <cglm/cglm.h>
#include <string.h>

#include "renderPassObj.h"
#include "engineCore.h"
#include "state.h"

#include "camera.h"
#include "myInstance.h"
#include "texture.h"

#include "objBuilder.h"
#include "gltfBuilder.h"
#include "fontBuilder.h"
#include "entity.h"

#include "system.h"

#include "pendulumEnum.h"

static void loadInput(struct EngineCore *this) {
    FILE *file = fopen("examples/input.txt", "r");

    struct system *p = malloc(sizeof(struct system));

#define LIGHTER { 0.4, 0.4, 1.0, 1.0 }
#define DARKER { 0.2, 0.2, 1.0, 1.0 }

    p->qMethod = 6;
    p->method = malloc(sizeof(struct method) * p->qMethod);

    p->method[0] = (struct method) {
        .name = "Euler",
        .coords = { 0.0, 0.0, 1.0 / 3.0, 0.5 },
        .color = LIGHTER,
        .method = euler
    };
    p->method[1] = (struct method) {
        .name = "Mod Euler",
        .coords = { 1.0 / 3.0, 0.0, 1.0 / 3.0, 0.5 },
        .color = DARKER,
        .method = modified_euler
    };
    p->method[2] = (struct method) {
        .name = "Heun",
        .coords = { 2.0 / 3.0, 0.0, 1.0 / 3.0, 0.5 },
        .color = LIGHTER,
        .method = heun
    };
    p->method[3] = (struct method) {
        .name = "RK4",
        .coords = { 0.0, 0.5, 1.0 / 3.0, 0.5 },
        .color = DARKER,
        .method = rk4
    };
    p->method[4] = (struct method) {
        .name = "RK5",
        .coords = { 1.0 / 3.0, 0.5, 1.0 / 3.0, 0.5 },
        .color = LIGHTER,
        .method = rk5
    };
    p->method[5] = (struct method) {
        .name = "20 x RK5",
        .coords = { 2.0 / 3.0, 0.5, 1.0 / 3.0, 0.5 },
        .color = DARKER,
        .method = x20rk5
    };

    fscanf(file, "%f", &p->time);
    fscanf(file, "%f %f %f", &p->pos[0], &p->pos[1], &p->pos[2]);
    fscanf(file, "%f %f", &p->tilt[0], &p->tilt[1]);

    fscanf(file, "%d %d", &p->pendulumCount, &p->nodeCount);

    struct variables (*var)[p->pendulumCount][p->nodeCount] = (void *)(p->var = malloc(sizeof(struct variables[p->pendulumCount][p->nodeCount]) * p->qMethod));
    struct params (*params)[p->pendulumCount][p->nodeCount] = (void *)(p->params = malloc(sizeof(struct params[p->pendulumCount][p->nodeCount]) * p->qMethod));

    for (int i = 0; i < p->pendulumCount; i += 1) {
        for (int j = 0; j < p->nodeCount; j += 1) {
            fscanf(file, "%lf %lf %lf %lf", 
                &var[0][i][j].th,
                &var[0][i][j].dth,
                &params[0][i][j].mass, 
                &params[0][i][j].length
            );
            var[0][i][j].th = glm_rad(var[0][i][j].th);
            for (int k = 1; k < p->qMethod; k += 1) {
                var[k][i][j] = var[0][i][j];
                params[k][i][j] = params[0][i][j];
            }
        }
    }

    fclose(file);

    addResource(&this->resource, PENDULUM_DATA, p, freeSystem);
}

#undef LIGHTER
#undef DARKER

static void addEntities(struct EngineCore *this) {
    struct ResourceManager *entityData = calloc(1, sizeof(struct ResourceManager));
    struct ResourceManager *modelData = findResource(&this->resource, MODEL);
    struct ResourceManager *objectLayouts = findResource(&this->resource, OBJECT_LAYOUTS);

    struct ResourceManager *nodeData = calloc(1, sizeof(struct ResourceManager));
    struct ResourceManager *lineData = calloc(1, sizeof(struct ResourceManager));
    struct ResourceManager *fontData = calloc(1, sizeof(struct ResourceManager));

    struct system *p = findResource(&this->resource, PENDULUM_DATA);

    struct descriptorSetLayout *objLayout = findResource(objectLayouts, OBJECT_LAYOUT_OBJ);
    struct descriptorSetLayout *gltfLayout = findResource(objectLayouts, OBJECT_LAYOUT_GLTF);
    struct descriptorSetLayout *fontLayout = findResource(objectLayouts, OBJECT_LAYOUT_FONT);

    for (int i = 0; i < p->qMethod; i += 1) {
        addResource(nodeData, i, createObj((struct ObjBuilder) {
            .instanceCount = (p->nodeCount + 1) * p->pendulumCount,
            .modelData = findResource(modelData, MODEL_SPHERE),
            .objectLayout = objLayout->descriptorSetLayout,

            INS(myInstance, myInstanceBuffer),
        }, &this->graphics), destroyEntity);

        addResource(lineData, i, createGltf((struct GltfBuilder) {
           .instanceCount = p->nodeCount * p->pendulumCount,
            .modelData = findResource(modelData, MODEL_LINE),
            .objectLayout = gltfLayout->descriptorSetLayout,

            INS(myInstance, myInstanceBuffer),
        }, &this->graphics), destroyEntity);

        addResource(fontData, i, createFont((struct FontBuilder) {
            .instanceCount = 1,
            .string = p->method[i].name,
            .modelData = findResource(modelData, MODEL_FONT),
            .objectLayout = fontLayout->descriptorSetLayout,

            INS(myInstance, myInstanceBuffer),
            .center = 0
        }, &this->graphics), destroyEntity);
    }

    addResource(entityData, ENTITY_NODE, nodeData, cleanupResourceManager);
    addResource(entityData, ENTITY_LINE, lineData, cleanupResourceManager);
    addResource(entityData, ENTITY_NAME, fontData, cleanupResourceManager);

    addResource(&this->resource, ENTITY, entityData, cleanupResourceManager);
}

void loadScreens(struct EngineCore *this) {
    struct ResourceManager *screenData = calloc(1, sizeof(struct ResourceManager));
    struct ResourceManager *screenModel = calloc(1, sizeof(struct ResourceManager));
    struct ResourceManager *screenText = calloc(1, sizeof(struct ResourceManager));

    struct ResourceManager *graphicsPipelineData = findResource(&this->resource, GRAPHIC_PIPELINE);
    struct ResourceManager *entityData = findResource(&this->resource, ENTITY);
    struct ResourceManager *renderPassCoreData = findResource(&this->resource, RENDER_PASS_CORE);
    struct ResourceManager *nodeData = findResource(entityData, ENTITY_NODE);
    struct ResourceManager *lineData = findResource(entityData, ENTITY_LINE);
    struct ResourceManager *nameData = findResource(entityData, ENTITY_NAME);
    struct system *p = findResource(&this->resource, PENDULUM_DATA);

    struct graphicsPipeline *pipe[] = {
        findResource(graphicsPipelineData, GRAPHIC_PIPELINE_OBJ),
        findResource(graphicsPipelineData, GRAPHIC_PIPELINE_GLTF),
        findResource(graphicsPipelineData, GRAPHIC_PIPELINE_FONT),
    };

    struct renderPassCore *renderPassArr[] = { 
        findResource(renderPassCoreData, RENDER_PASS_CLEAN),
        findResource(renderPassCoreData, RENDER_PASS_STAY)
    };

    struct Textures *colorTexture = findResource(findResource(&this->resource, TEXTURES), TEXTURES_COLOR);

    struct descriptorSetLayout *cameraLayout = findResource(findResource(&this->resource, OBJECT_LAYOUTS), OBJECT_LAYOUT_CAMERA);

    for (int i = 0; i < p->qMethod; i += 1) {
        addResource(screenModel, i, createRenderPassObj((struct renderPassBuilder){
            .renderPass = renderPassArr[0],
            .color = {
                p->method[i].color[0],
                p->method[i].color[1],
                p->method[i].color[2],
                p->method[i].color[3]
            },
            .coordinates = {
                p->method[i].coords[0],
                p->method[i].coords[1],
                p->method[i].coords[2],
                p->method[i].coords[3],
            },
            .data = (struct pipelineConnection[]) {
                {
                    .texture = &colorTexture->descriptor,
                    .pipe = pipe[0],
                    .entity = (struct Entity* []) {
                        findResource(nodeData, i),
                    },
                    .qEntity = 1
                },
                {
                    .texture = &colorTexture->descriptor,
                    .pipe = pipe[1],
                    .entity = (struct Entity* []) {
                        findResource(lineData, i),
                    },
                    .qEntity = 1
                },
            },
            .qData = 2,
            .updateCameraBuffer = myUpdateFirstPersonCameraBuffer,
            .cameraBufferSize = sizeof(struct CameraBuffer),
            .cameraSize = sizeof(struct camera),
            .camera = &(struct camera) {},
            .cameraDescriptorSetLayout = cameraLayout->descriptorSetLayout,
        }, &this->graphics), destroyRenderPassObj);
        addResource(screenText, i, createRenderPassObj((struct renderPassBuilder){
            .renderPass = renderPassArr[1],
            .coordinates = {
                p->method[i].coords[0],
                p->method[i].coords[1],
                p->method[i].coords[2],
                p->method[i].coords[3],
            },
            .color = {
                p->method[i].color[0],
                p->method[i].color[1],
                p->method[i].color[2],
                p->method[i].color[3]
            },
            .data = (struct pipelineConnection[]) {
                {
                    .pipe = pipe[2],
                    .entity = (struct Entity* []) {
                        findResource(nameData, i),
                    },
                    .qEntity = 1
                }
            },
            .qData = 1,
            .updateCameraBuffer = myUpdateFirstPersonCameraBuffer,
            .cameraBufferSize = sizeof(struct CameraBuffer),
            .cameraSize = sizeof(struct camera),
            .camera = &(struct camera) {},
            .cameraDescriptorSetLayout = cameraLayout->descriptorSetLayout,
        }, &this->graphics), destroyRenderPassObj);
        struct camera *camera[] = {
            ((struct renderPassObj *)findResource(screenModel, i))->camera,
            ((struct renderPassObj *)findResource(screenText, i))->camera,
        };

        for (size_t j = 0; j < sizeof(camera) / sizeof(struct camera *); j += 1) {
            camera[j]->pos[0] = p->pos[0];
            camera[j]->pos[1] = p->pos[1];
            camera[j]->pos[2] = p->pos[2];
            camera[j]->direction[0] = 0;
            camera[j]->direction[1] = -1;
            camera[j]->direction[2] = 0;
        };

        struct myInstance *text = ((struct Entity *)findResource(nameData, i))->instance;

        *text = (struct myInstance){
            .pos = { 0.0f, 0.3f, 0.0f },
            .rotation = { 0.0f, 0.0f, 0.0f },
            .fixedRotation = { 0.0f, 0.0f, 0.0f },
            .scale = { 7 * 10e-7, 7 * 10e-7, 7 * 10e-7 },
            .textureIndex = 0,
        };
    }

    addResource(screenData, SCREEN_MODEL, screenModel, cleanupResourceManager);
    addResource(screenData, SCREEN_TEXT, screenText, cleanupResourceManager);

    addResource(&this->resource, SCREEN_DATA, screenData, cleanupResourceManager);
}

void loadSimulation(struct EngineCore *engine, enum state *state) {
    loadInput(engine);
    addEntities(engine);

    loadScreens(engine);
    
    *state = SIMULATION;
}
