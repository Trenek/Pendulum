#ifndef UNIFORM_BUFFER_OBJECT_H
#define UNIFORM_BUFFER_OBJECT_H

#define GLM_FORCE_RADIANS
#include <cglm/cglm.h>

#include "cameraBuilder.h"

struct camera {
    vec3 pos;
    vec3 direction;
};

struct WindowManager;
typedef struct VkExtent2D VkExtent2D;

struct CameraBuffer {
    alignas(16) mat4 view;
    alignas(16) mat4 proj;
};

void moveCamera(struct WindowManager *windowControl, struct camera *camera, float deltaTime);

void myUpdateFirstPersonCameraBuffer(void *uniformBuffersMapped, VkExtent2D swapChainExtent, void *cameraPtr);

static inline struct cameraBuilder myFirstPersonCameraInfo(const struct camera * const restrict data) {
    return (struct cameraBuilder) {
        .updateBuffer = myUpdateFirstPersonCameraBuffer,
        .size = sizeof(struct camera),
        .bufferSize = sizeof(struct CameraBuffer),
        .mapped = data
    };
}

#endif
