#ifndef MODEL_BUILDER
#define MODEL_BUILDER
#include <vulkan/vulkan.h>

struct Model;

struct ModelBuilder {
    const char *modelPath;

    uint32_t texturesQuantity;
    const char **texturesPath;

    const char *vertexShader;
    const char *fragmentShader;

    uint32_t instanceCount;

    float minDepth;
    float maxDepth;

    struct ModelLoader {
        void (*loadModel)(const char *objectPath, struct Model *model, VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
    };

    struct ModelType {
        VkDescriptorSetLayout (*createDescriptorSetLayout)(VkDevice device, uint32_t textureQuantity);
        VkDescriptorPool (*createDescriptorPool)(uint32_t texturesCount, VkDevice device);
        void (*bindBuffersToDescriptorSets)(VkDescriptorSet descriptorSets[], VkDevice device, VkBuffer uniformBuffers[], struct Model model);
    };
};


#include "modelLoader.h"
struct ModelBuilder object(struct ModelBuilder a);
struct ModelBuilder interface(struct ModelBuilder a);

#endif
