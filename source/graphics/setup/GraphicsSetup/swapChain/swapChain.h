struct swapChain {
    VkImage *images;
    uint32_t imagesCount;
    VkFormat imageFormat;
    VkExtent2D extent;
    VkSwapchainKHR this;
};
