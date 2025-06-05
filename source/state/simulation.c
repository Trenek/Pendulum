#include <cglm.h>
#include <string.h>

#include <gsl/gsl_linalg.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>

#include "engineCore.h"
#include "state.h"

#include "entity.h"
#include "instanceBuffer.h"

#include "renderPassObj.h"

#include "pendulum.h"

#define g 9.81

void updatePos(struct instance *node, struct instance *line, struct node *params, int N) {
    node[0].pos[0] = +params[0].length * sin(params[0].th);
    node[0].pos[2] = -params[0].length * cos(params[0].th);
    for (int i = 1; i < N; i += 1) {
        node[i].pos[0] = node[i - 1].pos[0] + params[i].length * sin(params[i].th);
        node[i].pos[2] = node[i - 1].pos[2] - params[i].length * cos(params[i].th);
    }

    line[0].pos[0] = node[0].pos[0] / 2;
    line[0].pos[2] = node[0].pos[2] / 2;
    line[0].fixedRotation[1] = -params[0].th;
    for (int i = 0; i < N - 1; i += 1) {
        glm_vec3_lerp(node[i].pos, node[i + 1].pos, 0.5f, line[i + 1].pos);

        line[i + 1].fixedRotation[1] = -params[i + 1].th;
    }

    for (int i = 0; i < N; i += 1) {
        line[i].fixedRotation[1] += glm_rad(90);
    }
}

int sigma(int j, int k) {
    return j <= k;
}

int sign(int j, int k) {
    return j != k;
}

void fun(int n, struct node *init, double (*result)[2]) {
    double l[n]; for (int j = 0; j < n; j += 1) {
        l[j] = init[j].length;
    }
    double m[n]; for (int j = 0; j < n; j += 1) {
        m[j] = init[j].mass;
    }
    double th[n]; for (int j = 0; j < n; j += 1) {
        th[j] = init[j].th;
    }
    double dth[n]; for (int j = 0; j < n; j += 1) {
        dth[j] = init[j].dth;
    }

    double A[n][n] = {};
    double B[n] = {};

    gsl_matrix_view A_view = gsl_matrix_view_array((double *)A, n, n);
    gsl_vector_view b_view = gsl_vector_view_array(B, n);
    gsl_vector *x = gsl_vector_alloc(n);
    gsl_permutation *p = gsl_permutation_alloc(n);
    int signum = 0;

    for (int j = 0; j < n; j += 1) {
        for (int k = 0; k < n; k += 1) {
            float sum = 0;
            for (int q = k; q < n; q += 1) {
                if (sigma(j, q)) {
                    sum += m[q];
                }
            }
            sum *= l[j] * l[k];

            B[j] -= (
                sum * sin(th[j] - th[k]) * dth[j] * dth[k] +
                sum * sin(th[k] - th[j]) * (dth[j] - dth[k]) * dth[k]
            );

            if (sigma(j, k)) {
                B[j] -= g * l[j] * sin(th[j]) * m[k];
                A[j][j] += m[k] * pow(l[j], 2);
            }
            if (sign(j, k)) {
                A[j][k] += sum * cos(th[j] - th[k]);
            }
        }
    }

    gsl_linalg_LU_decomp(&A_view.matrix, p, &signum);
    gsl_linalg_LU_solve(&A_view.matrix, p, &b_view.vector, x);

    for (int j = 0; j < n; j += 1) {
        result[j][0] = init[j].dth;
        result[j][1] = gsl_vector_get(x, j);
    }

    gsl_vector_free(x);
    gsl_permutation_free(p);
}

void update(
    float t,
    struct system *s,
    struct instance *node[4][s->pendulumCount],
    struct instance *line[4][s->pendulumCount],
    void (**f)(int N, struct node *init, double t, void (*fun)(int n, struct node *init, double (*result)[2]))) {
    struct node (*nodee)[s->pendulumCount][s->nodeCount] = (void *)s->node;

    for (int i = 0; i < 4; i += 1)
    for (int j = 0; j < s->pendulumCount; j += 1) {
        f[i](s->nodeCount, nodee[i][j], t, fun);

        updatePos(node[i][j], line[i][j], nodee[i][j], s->nodeCount);
    }
}

void initNode(struct instance *node, struct instance *line, struct node params[], int N) {
    for (int i = 0; i < N + 1; i += 1) {
        glm_vec3_fill(node[i].pos, 0.0f);
        glm_vec3_fill(node[i].rotation, 0.0f);
        glm_vec3_fill(node[i].fixedRotation, 0.0f);
        glm_vec3_fill(node[i].scale, (i == N ? 1 : params[i].mass) / 10);
        node[i].textureIndex = 0;
        node[i].textureInc = 0;
        node[i].shadow = false;
    }

    for (int i = 0; i < N; i += 1) {
        glm_vec3_fill(line[i].pos, 0.0f);
        glm_vec3_fill(line[i].rotation, 0.0f);
        glm_vec3_fill(line[i].scale, 0.25f);
        glm_vec3_fill(line[i].fixedRotation, 0.0f);
        line[i].scale[1] = params[i].length;
        line[i].textureIndex = 1;
        line[i].textureInc = 0;
        line[i].shadow = false;
        line[i].fixedRotation[2] = glm_rad(-90);
        line[i].fixedRotation[1] = glm_rad(-90);
    }
}

void simulation(struct EngineCore *engine, enum state *state) {
    struct system *p = findResource(&engine->resource, "Pendulum");
    int M = p->pendulumCount;
    int N = p->nodeCount;
    struct node (*params)[M][N] = (void *)p->node;

    struct ResourceManager *entityData = findResource(&engine->resource, "Entity");
    struct ResourceManager *screenData = findResource(&engine->resource, "ScreenData");

    struct Entity *entity[] = {
        findResource(entityData, "Node1"),
        findResource(entityData, "Line1"),
        findResource(entityData, "Node2"),
        findResource(entityData, "Line2"),
        findResource(entityData, "Node3"),
        findResource(entityData, "Line3"),
        findResource(entityData, "Node4"),
        findResource(entityData, "Line4"),
    };
    size_t qEntity = sizeof(entity) / sizeof(struct Entity *);

    struct ResourceManager *renderPassCoreData = findResource(&engine->resource, "RenderPassCoreData");
    struct renderPassCore *renderPassArr[] = { 
        findResource(renderPassCoreData, "Clean"),
        findResource(renderPassCoreData, "Stay")
    };
    size_t qRenderPassArr = sizeof(renderPassArr) / sizeof(struct renderPassCore *);

    struct renderPassObj *renderPass[] = {
        findResource(screenData, "One"),
        findResource(screenData, "Two"),
        findResource(screenData, "Three"),
        findResource(screenData, "Four"),
    };
    size_t qRenderPass = sizeof(renderPass) / sizeof(struct renderPassObj *);

    struct instance *node[4][M];
    struct instance *line[4][M];

    for (int i = 0; i < 4; i += 1) {
        node[i][0] = entity[2 * i + 0]->instance;
        line[i][0] = entity[2 * i + 1]->instance;
        for (int j = 1; j < M; j += 1) {
            node[i][j] = node[i][j - 1] + entity[2 * i + 0]->instanceCount / M;
            line[i][j] = line[i][j - 1] + entity[2 * i + 1]->instanceCount / M;
        }

        for (int j = 0; j < M; j += 1) {
            initNode(node[i][j], line[i][j], params[i][j], N);
        }
    }

    bool isRunning = false;

    {
        struct node (*nodee)[p->pendulumCount][p->nodeCount] = (void *)p->node;
        for (int i = 0; i < 4; i += 1)
        for (int j = 0; j < p->pendulumCount; j += 1) {
            updatePos(node[i][j], line[i][j], nodee[i][j], p->nodeCount);
        }
    }

    while (*state == SIMULATION && !shouldWindowClose(engine->window)) {
        glfwPollEvents();

        void (*f[])(int, struct node *, double, void (*)(int n, struct node *, double (*)[2])) = {
            euler,
            heun,
            rk5,
            rk4
        };
        float dTime = p->time * engine->deltaTime.deltaTime;
        if (isRunning) update(dTime, p, node, line, f);

        updateInstances(entity, qEntity, dTime);
        moveCamera(&engine->window, engine->window.window, &renderPass[0]->camera, dTime);
        moveCamera(&engine->window, engine->window.window, &renderPass[1]->camera, dTime);
        moveCamera(&engine->window, engine->window.window, &renderPass[2]->camera, dTime);
        moveCamera(&engine->window, engine->window.window, &renderPass[3]->camera, dTime);

        drawFrame(engine, qRenderPass, renderPass, qRenderPassArr, renderPassArr);
        if ((KEY_PRESS | KEY_CHANGE) == getKeyState(&engine->window, GLFW_KEY_R)) {
            *state = LOAD_SIMULATION;
        }
        if ((KEY_PRESS | KEY_CHANGE) == getKeyState(&engine->window, GLFW_KEY_T)) {
            isRunning =! isRunning;
        }
    }

    switch (*state) {
        case LOAD_SIMULATION:
            vkDeviceWaitIdle(engine->graphics.device);
            cleanupResource(&engine->resource, "Pendulum");
            cleanupResource(&engine->resource, "Entity");
            cleanupResource(&engine->resource, "ScreenData");
            break;
        default:
    }
}
