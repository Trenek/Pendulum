#include <cglm.h>
#include <string.h>

#include "engineCore.h"
#include "state.h"

#include "asset.h"
#include "entity.h"
#include "instanceBuffer.h"

#include "graphicsPipelineObj.h"
#include "renderPassObj.h"

#include <string.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_linalg.h>

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

struct p {
    int pendulumCount;
    int nodeCount;

    struct node (*node)[];
    double (*y)[][2];
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
    f[1][1] = (-M2 * Lwws2 * cos(del) + (M1 + M2) * (Gs1 * cos(del) - Lwws1 - Gs2)) / (L2*den);

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
    line[0].fixedRotation[1] = -y[0][0];
    for (int i = 0; i < N - 1; i += 1) {
        glm_vec3_lerp(node[i].pos, node[i + 1].pos, 0.5f, line[i + 1].pos);

        line[i + 1].fixedRotation[1] = -y[i + 1][0];
    }

    for (int i = 0; i < N; i += 1) {
        line[i].fixedRotation[1] += glm_rad(90);
    }
}

void initNode(struct instance *node, struct instance *line, struct node params[], int N) {
    for (int i = 0; i < N + 1; i += 1) {
        glm_vec3_fill(node[i].pos, 0.0f);
        glm_vec3_fill(node[i].rotation, 0.0f);
        glm_vec3_fill(node[i].fixedRotation, 0.0f);
        glm_vec3_fill(node[i].scale, 0.1f);
        node[i].textureIndex = 0;
        node[i].textureInc = 0;
        node[i].shadow = false;
    }

    for (int i = 0; i < N; i += 1) {
        glm_vec3_fill(line[i].pos, 0.0f);
        glm_vec3_fill(line[i].rotation, 0.0f);
        glm_vec3_fill(line[i].scale, 0.25f);
        line[i].scale[1] = params[i].length;
        line[i].textureIndex = 1;
        line[i].textureInc = 0;
        line[i].shadow = false;
        glm_vec3_fill(line[i].fixedRotation, 0.0f);
        line[i].fixedRotation[2] = glm_rad(-90);
        line[i].fixedRotation[1] = glm_rad(-90);
    }
}

void game(struct EngineCore *engine, enum state *state) {
    struct p *p = findResource(&engine->resource, "Pendulum");
    int M = p->pendulumCount;
    int N = p->nodeCount;
    struct node (*params)[N] = p->node;
    double (*y)[N][2] = p->y;

    struct ResourceManager *graphicsPipelineData = findResource(&engine->resource, "graphicPipelines");
    struct ResourceManager *entityData = findResource(&engine->resource, "Entity");

    struct graphicsPipeline *pipe[] = {
        findResource(graphicsPipelineData, "Floor"),
    };

    struct Entity *entity[] = {
        findResource(entityData, "Node"),
        findResource(entityData, "Line"),
    };
    size_t qEntity = sizeof(entity) / sizeof(struct Entity *);

    struct ResourceManager *renderPassCoreData = findResource(&engine->resource, "RenderPassCoreData");
    struct renderPassCore *renderPassArr[] = { 
        findResource(renderPassCoreData, "Clean"),
        findResource(renderPassCoreData, "Stay")
    };
    size_t qRenderPassArr = sizeof(renderPassArr) / sizeof(struct renderPassCore *);

    struct renderPassObj *renderPass[] = {
        createRenderPassObj((struct renderPassBuilder){
            .renderPass = renderPassArr[0],
            .coordinates = { 0.0, 0.0, 1.0, 1.0 },
            .data = (struct pipelineConnection[]) {
                {
                    .pipe = pipe[0],
                    .entity = (struct Entity* []) {
                        entity[0],
                        entity[1],
                    },
                    .qEntity = 2
                },
            },
            .qData = 1,
            .updateCameraBuffer = updateFirstPersonCameraBuffer
        }, &engine->graphics),
    };
    size_t qRenderPass = sizeof(renderPass) / sizeof(struct renderPassObj *);

    double yy[M][N][2];

    struct instance *node[M];
    struct instance *line[M];

    node[0] = entity[0]->instance;
    line[0] = entity[1]->instance;
    for (int i = 1; i < M; i += 1) {
        node[i] = node[i - 1] + (entity[0]->instanceCount / M);
        line[i] = line[i - 1] + (entity[1]->instanceCount / M);
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

    renderPass[0]->camera = initCamera();
    glfwPollEvents();

    while (*state == GAME && !shouldWindowClose(engine->window)) {
        glfwPollEvents();

        memcpy(yy, y, sizeof(double[M][N][2]));
        for (int i = 0; i < M; i += 1) {
            gsl_odeiv2_driver_apply(driver[i], &t, t + engine->deltaTime.deltaTime, (void*)y[i]);
            updatePos(node[i], line[i], params[i], y[i], N);
            for (int j = 0; j < N; j += 1) {
                params[i][j].acc = (y[i][j][1] - yy[i][j][1]) / engine->deltaTime.deltaTime;
            }
        }

        updateInstances(entity, qEntity, engine->deltaTime.deltaTime);
        moveCamera(&engine->window, engine->window.window, &renderPass[0]->camera, engine->deltaTime.deltaTime);

        drawFrame(engine, qRenderPass, renderPass, qRenderPassArr, renderPassArr);
        if ((KEY_PRESS | KEY_CHANGE) == getKeyState(&engine->window, GLFW_KEY_R)) {
            vkDeviceWaitIdle(engine->graphics.device);
            cleanupResource(&engine->resource, "Pendulum");
            cleanupResource(&engine->resource, "Entity");

            *state = LOAD_GAME;
        }
    }

    vkDeviceWaitIdle(engine->graphics.device);
    destroyRenderPassObjArr(qRenderPass, renderPass);
}
