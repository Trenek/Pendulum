#include <vulkan/vulkan.h>

#include "definitions.h"
#include "swapChain.h"

struct GraphicsSetup {
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkSurfaceKHR surface;
    VkPhysicalDevice physicalDevice;
    VkQueue presentQueue;
    VkQueue graphicsQueue;
    VkQueue transferQueue;
    VkDevice device;

    struct swapChain swapChain;
    VkImageView *swapChainImageViews;

    VkRenderPass renderPass;

    VkFramebuffer *swapChainFramebuffers;

    VkCommandPool commandPool;
    VkCommandBuffer commandBuffer[MAX_FRAMES_IN_FLIGHT];
    VkCommandPool transferCommandPool;

    VkBuffer uniformBuffers[MAX_FRAMES_IN_FLIGHT];
    VkDeviceMemory uniformBuffersMemory[MAX_FRAMES_IN_FLIGHT];
    void *uniformBuffersMapped[MAX_FRAMES_IN_FLIGHT];

    VkSemaphore imageAvailableSemaphore[MAX_FRAMES_IN_FLIGHT];
    VkSemaphore renderFinishedSemaphore[MAX_FRAMES_IN_FLIGHT];
    VkFence inFlightFence[MAX_FRAMES_IN_FLIGHT];

    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;

    VkSampleCountFlagBits msaaSamples;

    VkImage colorImage;
    VkDeviceMemory colorImageMemory;
    VkImageView colorImageView;
};

void recreateSwapChainGraphics(GLFWwindow *window, struct GraphicsSetup *vulkan);
struct GraphicsSetup setupGraphics(GLFWwindow *window);
void cleanupGraphics(struct GraphicsSetup vulkan);
