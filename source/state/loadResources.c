#include <cglm/cglm.h>

#include "engineCore.h"
#include "state.h"

#include "texture.h"
#include "model.h"
#include "obj.h"
#include "objBuilder.h"
#include "font.h"
#include "fontBuilder.h"
#include "gltf.h"
#include "renderPassCore.h"

#include "defaultCamera.h"

#include "graphicsPipelineLayout.h"
#include "graphicsPipelineObj.h"
#include "descriptorSetLayoutObj.h"

#include "pendulumEnum.h"

static void addTextures(struct EngineCore *this) {
    struct ResourceManager *textureManager = calloc(1, sizeof(struct ResourceManager));

    addResource(textureManager, TEXTURES_COLOR, loadTextures(&this->graphics, 2, (struct TextureData[]){
        { .data = "textures/green.jpg" },
        { .data = "textures/red.jpg" },
    }), unloadTextures);

    addResource(&this->resource, TEXTURES, textureManager, cleanupResourceManager);
}

static void addModelData(struct EngineCore *this) {
    struct ResourceManager *modelData = calloc(1, sizeof(struct ResourceManager));

    addResource(modelData, MODEL_SPHERE, loadModel("models/sphere.obj", &this->graphics), destroyActualModel);
    addResource(modelData, MODEL_LINE, loadModel("models/cylinder.glb", &this->graphics), destroyActualModel);
    addResource(modelData, MODEL_FONT, loadModel("fonts/c.ttf", &this->graphics), destroyActualModel);

    addResource(&this->resource, MODEL, modelData, cleanupResourceManager);
}

static void addRenderPassCoreData(struct EngineCore *this) {
    struct ResourceManager *renderPassCoreData = calloc(1, sizeof(struct ResourceManager));

    addResource(renderPassCoreData, RENDER_PASS_CLEAN, createRenderPassCore((struct renderPassCoreBuilder) {
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .initLayout = VK_IMAGE_LAYOUT_UNDEFINED
    }, &this->graphics), freeRenderPassCore);
    addResource(renderPassCoreData, RENDER_PASS_STAY, createRenderPassCore((struct renderPassCoreBuilder) {
        .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
        .initLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    }, &this->graphics), freeRenderPassCore);

    addResource(&this->resource, RENDER_PASS_CORE, renderPassCoreData, cleanupResourceManager);
}

static void addObjectLayout(struct EngineCore *this) {
    struct ResourceManager *objectLayoutData = calloc(1, sizeof(struct ResourceManager));

    addResource(objectLayoutData, OBJECT_LAYOUT_OBJ,
        defaultObjDescriptorSetLayout(this->graphics.device),
        destroyDescriptorSetLayout
    );
    addResource(objectLayoutData, OBJECT_LAYOUT_GLTF,
        defaultGltfDescriptorSetLayout(this->graphics.device),
        destroyDescriptorSetLayout
    );
    addResource(objectLayoutData, OBJECT_LAYOUT_FONT,
        defaultFontDescriptorSetLayout(this->graphics.device),
        destroyDescriptorSetLayout
    );
    addResource(objectLayoutData, OBJECT_LAYOUT_CAMERA,
        defaultCameraDescriptorSetLayout(this->graphics.device),
        destroyDescriptorSetLayout
    );

    addResource(&this->resource, OBJECT_LAYOUTS, objectLayoutData, cleanupResourceManager);
}

static void createGraphicPipelineLayouts(struct EngineCore *this) {
    struct ResourceManager *graphicPipelinesData = calloc(1, sizeof(struct ResourceManager));

    struct ResourceManager *objectData = findResource(&this->resource, OBJECT_LAYOUTS);
    struct ResourceManager *textureData = findResource(&this->resource, TEXTURES);

    struct Textures *colorTexture = findResource(textureData, TEXTURES_COLOR);

    struct descriptorSetLayout *gltfLayout = findResource(objectData, OBJECT_LAYOUT_GLTF);
    struct descriptorSetLayout *objectLayout = findResource(objectData, OBJECT_LAYOUT_OBJ);
    struct descriptorSetLayout *fontLayout = findResource(objectData, OBJECT_LAYOUT_FONT);
    struct descriptorSetLayout *cameraLayout = findResource(objectData, OBJECT_LAYOUT_CAMERA);

    addResource(graphicPipelinesData, GRAPHIC_PIPELINE_LAYOUT_OBJ, createGraphicPipelineLayout((struct graphicsPipelineLayoutBuilder) {
        .descriptorSetLayout = (VkDescriptorSetLayout []){
            cameraLayout->descriptorSetLayout,
            colorTexture->descriptor.descriptorSetLayout,
            objectLayout->descriptorSetLayout,
        },
        .qDescriptorSetLayout = 3,

        .pushConstantRangeCount = 1,
        .pPushConstantRanges = (VkPushConstantRange []) {
            {
                .offset = 0,
                .size = sizeof(struct ObjPushConstants),
                .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
            }
        }
    }, &this->graphics), destroyPipelineLayoutObj);
    addResource(graphicPipelinesData, GRAPHIC_PIPELINE_LAYOUT_GLTF, createGraphicPipelineLayout((struct graphicsPipelineLayoutBuilder) {
        .descriptorSetLayout = (VkDescriptorSetLayout []){
            cameraLayout->descriptorSetLayout,
            colorTexture->descriptor.descriptorSetLayout,
            gltfLayout->descriptorSetLayout,
        },
        .qDescriptorSetLayout = 3,

        .pushConstantRangeCount = 1,
        .pPushConstantRanges = (VkPushConstantRange []) {
            {
                .offset = 0,
                .size = sizeof(struct GltfPushConstants),
                .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT
            }
        }
    }, &this->graphics), destroyPipelineLayoutObj);
    addResource(graphicPipelinesData, GRAPHIC_PIPELINE_LAYOUT_FONT, createGraphicPipelineLayout((struct graphicsPipelineLayoutBuilder) {
        .descriptorSetLayout = (VkDescriptorSetLayout []){
            cameraLayout->descriptorSetLayout,
            fontLayout->descriptorSetLayout,
        },
        .qDescriptorSetLayout = 2,

        .pushConstantRangeCount = 1,
        .pPushConstantRanges = (VkPushConstantRange []) {
            {
                .offset = 0,
                .size = sizeof(struct FontPushConstants),
                .stageFlags = VK_SHADER_STAGE_VERTEX_BIT
            }
        }
    }, &this->graphics), destroyPipelineLayoutObj);

    addResource(&this->resource, GRAPHIC_PIPELINE_LAYOUTS, graphicPipelinesData, cleanupResourceManager);
}

static void createGraphicPipelines(struct EngineCore *this) {
    struct ResourceManager *graphicPipelinesData = calloc(1, sizeof(struct ResourceManager));
    struct ResourceManager *renderPassCoreData = findResource(&this->resource, RENDER_PASS_CORE);
    struct ResourceManager *graphicPipelineLayouts = findResource(&this->resource, GRAPHIC_PIPELINE_LAYOUTS);

    struct renderPassCore *renderPass[] = {
        findResource(renderPassCoreData, RENDER_PASS_CLEAN),
        findResource(renderPassCoreData, RENDER_PASS_STAY)
    };

    struct graphicsPipelineLayout *objLayout = findResource(graphicPipelineLayouts, GRAPHIC_PIPELINE_LAYOUT_OBJ);
    struct graphicsPipelineLayout *gltfLayout = findResource(graphicPipelineLayouts, GRAPHIC_PIPELINE_LAYOUT_GLTF);
    struct graphicsPipelineLayout *fontLayout = findResource(graphicPipelineLayouts, GRAPHIC_PIPELINE_LAYOUT_FONT);

    size_t qRenderPass = sizeof(renderPass) / sizeof(struct renderPassCore *);

    addResource(graphicPipelinesData, GRAPHIC_PIPELINE_OBJ, createGraphicsPipelineObj((struct GraphicsPipelineBuilder) {
        .pipelineLayout = objLayout->pipelineLayout,
        .qRenderPassCore = qRenderPass,
        .renderPassCore = renderPass,
        .vertexShader = "shaders/objVert.spv",
        .fragmentShader = "shaders/frag.spv",
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,

        .vert = defaultObjVert(),
        .operation = VK_COMPARE_OP_LESS,
        .cullFlags = VK_CULL_MODE_BACK_BIT,
    }, &this->graphics), destroyPipelineObj);
    addResource(graphicPipelinesData, GRAPHIC_PIPELINE_GLTF, createGraphicsPipelineObj((struct GraphicsPipelineBuilder) {
        .pipelineLayout = gltfLayout->pipelineLayout,
        .qRenderPassCore = qRenderPass,
        .renderPassCore = renderPass,
        .vertexShader = "shaders/gltfVert.spv",
        .fragmentShader = "shaders/frag.spv",
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,

        .vert = defaultGltfVert(),
        .operation = VK_COMPARE_OP_LESS,
        .cullFlags = VK_CULL_MODE_BACK_BIT,
    }, &this->graphics), destroyPipelineObj);
    addResource(graphicPipelinesData, GRAPHIC_PIPELINE_FONT, createGraphicsPipelineObj((struct GraphicsPipelineBuilder) {
        .pipelineLayout = fontLayout->pipelineLayout,
        .qRenderPassCore = qRenderPass,
        .renderPassCore = renderPass,
        .vertexShader = "shaders/text2dV.spv",
        .fragmentShader = "shaders/textF.spv",
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,

        .vert = defaultFontVert(),
        .operation = VK_COMPARE_OP_LESS,
        .cullFlags = VK_CULL_MODE_BACK_BIT,
    }, &this->graphics), destroyPipelineObj);

    addResource(&this->resource, GRAPHIC_PIPELINE, graphicPipelinesData, cleanupResourceManager);
}

void loadResources(struct EngineCore *engine, enum state *state) {
    addTextures(engine);
    addModelData(engine);

    addRenderPassCoreData(engine);
    addObjectLayout(engine);

    createGraphicPipelineLayouts(engine);
    createGraphicPipelines(engine);

    *state = LOAD_SIMULATION;
}
