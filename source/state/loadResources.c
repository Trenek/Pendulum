#include <cglm.h>

#include "engineCore.h"
#include "state.h"

#include "asset.h"
#include "entity.h"
#include "modelBuilder.h"
#include "stringBuilder.h"

#include "renderPassCore.h"

#include "graphicsPipelineObj.h"

#include "Vertex.h"

static void addTextures(struct EngineCore *this) {
    struct ResourceManager *textureManager = calloc(1, sizeof(struct ResourceManager));

    addResource(textureManager, "Color", loadTextures(&this->graphics, 2, (const char *[]){
        "textures/green.jpg",
        "textures/red.jpg",
    }), unloadTextures);

    addResource(&this->resource, "textures", textureManager, cleanupResources);
}

static void addModelData(struct EngineCore *this) {
    struct ResourceManager *modelData = calloc(1, sizeof(struct ResourceManager));

    addResource(modelData, "sphere", loadModel("models/sphere.obj", &this->graphics), destroyActualModel);
    addResource(modelData, "line", loadModel("models/cylinder.glb", &this->graphics), destroyActualModel);

    addResource(&this->resource, "modelData", modelData, cleanupResources);
}

static void addRenderPassCoreData(struct EngineCore *this) {
    struct ResourceManager *renderPassCoreData = calloc(1, sizeof(struct ResourceManager));

    addResource(renderPassCoreData, "Clean", createRenderPassCore((struct renderPassCoreBuilder) {
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .initLayout = VK_IMAGE_LAYOUT_UNDEFINED
    }, &this->graphics), freeRenderPassCore);
    addResource(renderPassCoreData, "Stay", createRenderPassCore((struct renderPassCoreBuilder) {
        .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
        .initLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    }, &this->graphics), freeRenderPassCore);

    addResource(&this->resource, "RenderPassCoreData", renderPassCoreData, cleanupResources);
}

static void addObjectLayout(struct EngineCore *this) {
    struct ResourceManager *objectLayoutData = calloc(1, sizeof(struct ResourceManager));

    addResource(objectLayoutData, "object", createDescriptorSetLayout(
        createObjectDescriptorSetLayout(this->graphics.device, 2, (VkDescriptorSetLayoutBinding []) {
            {
                .binding = 0,
                .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                .descriptorCount = 1,
                .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
                .pImmutableSamplers = NULL
            },
            {
                .binding = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                .descriptorCount = 1,
                .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
                .pImmutableSamplers = NULL
            }
        }), this->graphics.device), 
        destroyDescriptorSetLayout
    );
    addResource(objectLayoutData, "camera", 
        createDescriptorSetLayout(createCameraDescriptorSetLayout(this->graphics.device), this->graphics.device), 
        destroyDescriptorSetLayout
    );

    addResource(&this->resource, "objectLayout", objectLayoutData, cleanupResources);
}

static void createGraphicPipelines(struct EngineCore *this) {
    struct ResourceManager *graphicPipelinesData = calloc(1, sizeof(struct ResourceManager));
    struct ResourceManager *renderPassCoreData = findResource(&this->resource, "RenderPassCoreData");

    struct Textures *colorTexture = findResource(findResource(&this->resource, "textures"), "Color");
    struct descriptorSetLayout *objectLayout = findResource(findResource(&this->resource, "objectLayout"), "object");
    struct descriptorSetLayout *cameraLayout = findResource(findResource(&this->resource, "objectLayout"), "camera");

    struct renderPassCore *renderPass[] = {
        findResource(renderPassCoreData, "Clean"),
        findResource(renderPassCoreData, "Stay")
    };
    size_t qRenderPass = sizeof(renderPass) / sizeof(struct renderPassCore *);

    addResource(graphicPipelinesData, "Floor", createObjGraphicsPipeline((struct graphicsPipelineBuilder) {
        .qRenderPassCore = qRenderPass,
        .renderPassCore = renderPass,
        .vertexShader = "shaders/vert.spv",
        .fragmentShader = "shaders/frag2.spv",
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,

        .texture = &colorTexture->descriptor,
        .objectLayout = objectLayout->descriptorSetLayout,

        Vert(AnimVertex),
        .operation = VK_COMPARE_OP_LESS,
        .cullFlags = VK_CULL_MODE_BACK_BIT,

        .cameraLayout = cameraLayout->descriptorSetLayout
    }, &this->graphics), destroyObjGraphicsPipeline);

    addResource(&this->resource, "graphicPipelines", graphicPipelinesData, cleanupResources);
}

static void loadSounds(struct EngineCore *) {}

void loadResources(struct EngineCore *engine, enum state *state) {
    addTextures(engine);
    addModelData(engine);

    addRenderPassCoreData(engine);
    addObjectLayout(engine);

    createGraphicPipelines(engine);

    loadSounds(engine);

    *state = LOAD_SIMULATION;
}
