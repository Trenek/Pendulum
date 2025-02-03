#include <vulkan/vulkan_core.h>

#include "uniformBufferObject.h"
#include "instanceBuffer.h"
#include "model.h"
#include "modelBuilder.h"
#include "MY_ASSERT.h"
#include "definitions.h"

static VkDescriptorPool createDescriptorPool(uint32_t texturesCount, VkDevice device) {
    VkDescriptorPool descriptorPool = NULL;

    VkDescriptorPoolSize poolSize[] = {
        [0] = {
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = MAX_FRAMES_IN_FLIGHT
        },
        [1] = {
            .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = texturesCount * MAX_FRAMES_IN_FLIGHT
        },
        [2] = {
            .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .descriptorCount = MAX_FRAMES_IN_FLIGHT * 2
        }
    };

    VkDescriptorPoolCreateInfo poolInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .poolSizeCount = sizeof(poolSize) / sizeof(VkDescriptorPoolSize),
        .pPoolSizes = poolSize,
        .maxSets = MAX_FRAMES_IN_FLIGHT,
        .flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT
    };

    MY_ASSERT(VK_SUCCESS == vkCreateDescriptorPool(device, &poolInfo, NULL, &descriptorPool));

    return descriptorPool;
}

static VkDescriptorSetLayout createDescriptorSetLayout(VkDevice device, uint32_t textureQuantity) {
    VkDescriptorSetLayout descriptorSetLayout = NULL;

    VkDescriptorSetLayoutBinding uboLayoutBinding = {
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        .pImmutableSamplers = NULL
    };

    VkDescriptorSetLayoutBinding samplerLayoutBinding = {
        .binding = 1,
        .descriptorCount = textureQuantity,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .pImmutableSamplers = NULL,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
    };

    VkDescriptorSetLayoutBinding uboModelLayoutBinding = {
        .binding = 2,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        .pImmutableSamplers = NULL
    };

    VkDescriptorSetLayoutBinding uboMeshLayoutBinding = {
        .binding = 3,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        .pImmutableSamplers = NULL
    };

    VkDescriptorSetLayoutBinding bindings[] = {
        uboLayoutBinding,
        samplerLayoutBinding,
        uboModelLayoutBinding,
        uboMeshLayoutBinding
    };

    VkDescriptorSetLayoutCreateInfo layoutInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = sizeof(bindings) / sizeof(VkDescriptorSetLayoutBinding),
        .pBindings = bindings,
    };

    MY_ASSERT(VK_SUCCESS == vkCreateDescriptorSetLayout(device, &layoutInfo, NULL, &descriptorSetLayout));

    return descriptorSetLayout;
}

static void bindBuffersToDescriptorSets(VkDescriptorSet descriptorSets[], VkDevice device, VkBuffer uniformBuffers[], struct Model model) {
    VkDescriptorImageInfo imageInfoArray[model.texturesQuantity];
    for (uint32_t i = 0; i < model.texturesQuantity; i += 1) {
        imageInfoArray[i] = (VkDescriptorImageInfo) {
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            .imageView = model.texture[i].imageView,
            .sampler = model.texture[i].sampler
        };
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i += 1) {
        VkDescriptorBufferInfo bufferInfo = {
            .buffer = uniformBuffers[i],
            .offset = 0,
            .range = sizeof(struct UniformBufferObject)
        };

        VkDescriptorBufferInfo modelBufferInfo[] = {
            {
                .buffer = model.graphics.uniformModelBuffers[i],
                .offset = 0,
                .range = model.instanceCount * sizeof(struct instanceBuffer)
            }
        };

        VkDescriptorBufferInfo meshBufferInfo[] = {
            {
                .buffer = model.graphics.localMeshBuffers[i],
                .offset = 0,
                .range = model.meshQuantity * sizeof(mat4)
            }
        };

        VkWriteDescriptorSet descriptorWrites[] = {
            [0] = {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = descriptorSets[i],
                .dstBinding = 0,
                .dstArrayElement = 0,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .descriptorCount = 1,
                .pBufferInfo = &bufferInfo,
                .pTexelBufferView = NULL
            },
            [1] = {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = descriptorSets[i],
                .dstBinding = 1,
                .dstArrayElement = 0,
                .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .descriptorCount = sizeof(imageInfoArray) / sizeof(VkDescriptorImageInfo),
                .pImageInfo = imageInfoArray,
                .pTexelBufferView = NULL
            },
            [2] = {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = descriptorSets[i],
                .dstBinding = 2,
                .dstArrayElement = 0,
                .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                .descriptorCount = sizeof(modelBufferInfo) / sizeof(VkDescriptorBufferInfo),
                .pBufferInfo = modelBufferInfo,
                .pTexelBufferView = NULL
            },
            [3] = {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = descriptorSets[i],
                .dstBinding = 3,
                .dstArrayElement = 0,
                .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                .descriptorCount = sizeof(meshBufferInfo) / sizeof(VkDescriptorBufferInfo),
                .pBufferInfo = meshBufferInfo,
                .pTexelBufferView = NULL
            }
        };

        vkUpdateDescriptorSets(device, sizeof(descriptorWrites) / sizeof(VkWriteDescriptorSet), descriptorWrites, 0, NULL);
    }
}

struct ModelBuilder object(struct ModelBuilder a) {
    a.createDescriptorPool = createDescriptorPool;
    a.createDescriptorSetLayout = createDescriptorSetLayout;
    a.bindBuffersToDescriptorSets = bindBuffersToDescriptorSets;

    return a;
}
