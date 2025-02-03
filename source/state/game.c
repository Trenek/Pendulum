#include <string.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_linalg.h>

#include "VulkanTools.h"

#include "model.h"
#include "modelBuilder.h"
#include "instanceBuffer.h"

#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_odeiv2.h>

#define g 9.81

struct node {
    double mass;
    double length;
    double acc;
};

int singlePendulum([[maybe_unused]]double t, const double y[], double f[], void *params) {
    struct node *info = params;

    f[0] = y[1];
    f[1] = -(g / info->length) * sin(y[0]);

    return GSL_SUCCESS;
}

int doublePendulum([[maybe_unused]]double t, const double y[], double ff[], void *params) {
    double th1 = y[0], w1 = y[1];
    double th2 = y[2], w2 = y[3];

    struct node *info = params;

    double M1 = info[0].mass;
    double M2 = info[1].mass;
    double L1 = info[0].length;
    double L2 = info[1].length;

    double (*f)[2] = (double (*)[2])ff;
    f[0][0] = w1;
    f[1][0] = w2;

    double del = th2 - th1;
    double den = (M1 + M2) - M2 * cos(del) * cos(del);
    double Lwws1 = L1 * (w1*w1) * sin(del);
    double Lwws2 = L2 * (w2*w2) * sin(del);
    double Gs1 = g*sin(th1), Gs2 = g*sin(th2);

    f[0][1] = (M2 * (Lwws1 + Gs2) * cos(del) + M2 * Lwws2 - (M1 + M2) * Gs1) / (L1*den);
    f[1][1] = (-M2 * Lwws2 * cos(del) + (M1 + M2) * ( Gs1 * cos(del) - Lwws1 - Gs2)) / (L2*den);

    return GSL_SUCCESS;
}

void updatePos(struct instance *node, struct instance *line, struct node *params, double y[][2], int N) {
    node[0].pos[0] = params[0].length * sin(y[0][0]);
    node[0].pos[2] = - params[0].length * cos(y[0][0]);
    for (int i = 1; i < N; i += 1) {
        node[i].pos[0] = node[i - 1].pos[0] + params[i].length * sin(y[i][0]);
        node[i].pos[2] = node[i - 1].pos[2] - params[i].length * cos(y[i][0]);
    }

    line[0].pos[0] = node[0].pos[0] / 2;
    line[0].pos[2] = node[0].pos[2] / 2;
    line[0].baseRotation[1] = -y[0][0];
    for (int i = 0; i < N - 1; i += 1) {
        glm_vec3_lerp(node[i].pos, node[i + 1].pos, 0.5f, line[i + 1].pos);

        line[i + 1].baseRotation[1] = -y[i + 1][0];
    }
}

void initNode(struct instance *node, struct instance *line, struct node params[], int N) {
    for (int i = 0; i < N + 1; i += 1) {
        glm_vec3_fill(node[i].pos, 0.0f);
        glm_vec3_fill(node[i].rotation, 0.0f);
        glm_vec3_fill(node[i].baseRotation, 0.0f);
        glm_vec3_fill(node[i].scale, 0.1f);
        node[i].textureIndex = 0;
        node[i].textureInc = 0;
        node[i].shadow = false;
    }

    for (int i = 0; i < N; i += 1) {
        glm_vec3_fill(line[i].pos, 0.0f);
        glm_vec3_fill(line[i].rotation, 0.0f);
        glm_vec3_fill(line[i].baseRotation, 0.0f);
        glm_vec3_fill(line[i].scale, 0.25f);
        line[i].scale[1] = params[i].length;
        line[i].textureIndex = 0;
        line[i].textureInc = 0;
        line[i].shadow = false;
    }
}

struct Model createPendulumNodes(struct GraphicsSetup *graphics, int N, int M) {
    return createModels(object(objLoader((struct ModelBuilder) {
        .instanceCount = (N + 1) * M,
        .texturesQuantity = 1,
        .texturesPath = (const char *[]){
            "textures/green.jpg",
        },
        .modelPath = "models/sphere.obj",
        .vertexShader = "shaders/vert.spv",
        .fragmentShader = "shaders/frag2.spv",
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    })), graphics);
}

struct Model createPendulumLines(struct GraphicsSetup *graphics, int N, int M) {
    return createModels(object(gltfLoader((struct ModelBuilder) {
        .instanceCount = N * M,
        .texturesQuantity = 1,
        .texturesPath = (const char *[]){
            "textures/red.jpg",
        },
        .modelPath = "models/cylinder.glb",
        .vertexShader = "shaders/vert.spv",
        .fragmentShader = "shaders/frag2.spv",
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    })), graphics);
}

void game(struct VulkanTools *vulkan, int M, int N, struct node params[M][N], double y[M][N][2]) {
    struct Model model[] = {
        createPendulumNodes(&vulkan->graphics, N, M),
        createPendulumLines(&vulkan->graphics, N, M)
    };

    double yy[M][N][2];

    struct instance *node[M];
    struct instance *line[M];

    node[0] = model[0].instance;
    line[0] = model[1].instance;
    for (int i = 1; i < M; i += 1) {
        node[i] = node[i - 1] + (model[0].instanceCount / M);
        line[i] = line[i - 1] + (model[1].instanceCount / M);
    }

    for (int i = 0; i < M; i += 1) {
        initNode(node[i], line[i], params[i], N);
    }

    double t = 0;
    double dt = 0.01;

    gsl_odeiv2_system sys[M];
    for (int i = 0; i < M; i += 1) {
        sys[i] = (gsl_odeiv2_system){
            .function = N == 1 ? singlePendulum : doublePendulum,
            .jacobian = NULL,
            .dimension = 2 * N,
            .params = params[i]
        };
    }
    gsl_odeiv2_driver *driver[M];
    for (int i = 0; i < M; i += 1) {
        driver[i] = gsl_odeiv2_driver_alloc_y_new(&sys[i], gsl_odeiv2_step_rk8pd, dt, 1e-6, 1e-6);
    }

    vulkan->camera = initCamera();
    glfwPollEvents();
    while (GLFW_PRESS != glfwGetKey(vulkan->window, GLFW_KEY_R) && !glfwWindowShouldClose(vulkan->window)) {
        drawFrame(vulkan, sizeof(model) / sizeof(struct Model), model);

        memcpy(yy, y, sizeof(double[M][N][2]));
        for (int i = 0; i < M; i += 1) {
            gsl_odeiv2_driver_apply(driver[i], &t, t + vulkan->deltaTime.deltaTime, (void*)y[i]);
            updatePos(node[i], line[i], params[i], y[i], N);
            for (int j = 0; j < N; j += 1) {
                params[i][j].acc = (y[i][j][1] - yy[i][j][1]) / vulkan->deltaTime.deltaTime;
            }
        }

        moveCamera(vulkan->windowControl, vulkan->window, vulkan->camera.center, vulkan->camera.cameraPos, vulkan->camera.tilt, vulkan->deltaTime.deltaTime);

        glfwPollEvents();
    }

    destroyModelArray(sizeof(model) / sizeof(struct Model), model, &vulkan->graphics);
}
