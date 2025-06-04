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
    node[0].pos[0] = +params[0].length * sin(params[0].angle);
    node[0].pos[2] = -params[0].length * cos(params[0].angle);
    for (int i = 1; i < N; i += 1) {
        node[i].pos[0] = node[i - 1].pos[0] + params[i].length * sin(params[i].angle);
        node[i].pos[2] = node[i - 1].pos[2] - params[i].length * cos(params[i].angle);
    }

    line[0].pos[0] = node[0].pos[0] / 2;
    line[0].pos[2] = node[0].pos[2] / 2;
    line[0].fixedRotation[1] = -params[0].angle;
    for (int i = 0; i < N - 1; i += 1) {
        glm_vec3_lerp(node[i].pos, node[i + 1].pos, 0.5f, line[i + 1].pos);

        line[i + 1].fixedRotation[1] = -params[i + 1].angle;
    }

    for (int i = 0; i < N; i += 1) {
        line[i].fixedRotation[1] += glm_rad(90);
    }
}

// θ`` + g * sin(θ) / l = 0
// 
// θ_1 = θ
// θ_2 = θ`
//
// θ_1` = θ_2
// θ_2` = - g * sin(θ_1)
void fun1(struct node *init, double (*result)[2]) {
    result[0][0] = init[0].angularVelocity;
    result[0][1] = -g * sin(init[0].angle);
}

void fun21(struct node *initPtr, double (*result)[2]) {
    struct node *init = initPtr + 1;

    double A = (init[1].mass + init[2].mass) * init[1].length * init[1].length;
    double B = init[2].mass * init[1].length * init[2].length * cos(init[1].angle - init[2].angle);
    double C = (
        init[2].mass * init[1].length * init[2].length * init[2].angularVelocity * init[2].angularVelocity * sin(init[1].angle - init[2].angle) +
        (init[1].mass + init[2].mass) * init[1].length * g * sin(init[1].angle)
    );
    double D = init[2].mass * init[2].length * init[2].length;
    double E = init[2].mass * init[1].length * init[2].length * cos(init[1].angle - init[2].angle);
    double F = (
        init[2].mass * init[2].length * g * sin(init[2].angle) -
        init[2].mass * init[1].length * init[2].length * init[1].angularVelocity * init[1].angularVelocity * sin(init[1].angle - init[2].angle)
    );

    double AArr[] = {
        A, B,
        D, E
    };
    double BArr[] = {
        C,
        F
    };

    gsl_matrix_view A_view = gsl_matrix_view_array(AArr, 2, 2);
    gsl_vector_view b_view = gsl_vector_view_array(BArr, 2);
    gsl_vector *x = gsl_vector_alloc(2);
    gsl_permutation *p = gsl_permutation_alloc(2);
    int signum = 0;

    gsl_linalg_LU_decomp(&A_view.matrix, p, &signum);
    gsl_linalg_LU_solve(&A_view.matrix, p, &b_view.vector, x);

    result[0][0] = init[1].angularVelocity;
    result[0][1] = gsl_vector_get(x, 0);
    result[1][0] = init[2].angularVelocity;
    result[1][1] = gsl_vector_get(x, 1);

    gsl_vector_free(x);
    gsl_permutation_free(p);
}

void fun2(struct node *init, double (*result)[2]) {
    double deltaT = init[0].angle - init[1].angle;
    double alfa = init[0].mass + init[1].mass * sin(deltaT) * sin(deltaT);
    double M = init[0].mass + init[1].mass;

    result[0][0] = fmod(init[0].angularVelocity, 2 * M_PI);
    result[0][1] = (
        -sin(deltaT) * init[1].mass * (
            init[0].length * init[0].angularVelocity * init[0].angularVelocity * cos(deltaT) +
            init[1].length * init[1].angularVelocity * init[1].angularVelocity
        ) - g * (
            M  * sin(init[0].angle) -
            init[1].mass * sin(init[1].angle) * cos(deltaT)
        )
    ) / (alfa * init[0].length);

    result[1][0] = fmod(init[1].angularVelocity, 2 * M_PI);
    result[1][1] = (
        sin(deltaT) * (
            M * init[0].length * init[0].angularVelocity * init[0].angularVelocity +
            init[1].mass * init[1].length * init[1].angularVelocity * init[1].angularVelocity * cos(deltaT)
        ) + g * (
            M  * sin(init[0].angle) * cos(deltaT) -
            M * sin(init[1].angle)
        )
    ) / (alfa * init[1].length);
}

void update(
    float t,
    struct system *s,
    struct instance *node[4][s->pendulumCount],
    struct instance *line[4][s->pendulumCount],
    void (**f)(int N, struct node *init, double t, void (*fun)(struct node *init, double (*result)[2]))) {
    struct node (*nodee)[s->pendulumCount][s->nodeCount] = (void *)s->node;
    void (*fun[])(struct node *init, double (*result)[2]) = {
        fun1,
        fun21
    };

    for (int i = 0; i < 4; i += 1)
    for (int j = 0; j < s->pendulumCount; j += 1) {
        f[i](s->nodeCount, nodee[i][j], t, fun[s->nodeCount - 1]);

        updatePos(node[i][j], line[i][j], nodee[i][j], s->nodeCount);
    }
}

void initNode(struct instance *node, struct instance *line, struct node params[], int N) {
    for (int i = 0; i < N + 1; i += 1) {
        glm_vec3_fill(node[i].pos, 0.0f);
        glm_vec3_fill(node[i].rotation, 0.0f);
        glm_vec3_fill(node[i].fixedRotation, 0.0f);
        glm_vec3_fill(node[i].scale, (i == 2 ? 1 : params[i].mass) / 10);
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

        void (*f[])(int, struct node *, double, void (*)(struct node *, double (*)[2])) = {
            euler,
            heun,
            modified_euler,
            rk4
        };
        if (isRunning) update(engine->deltaTime.deltaTime, p, node, line, f);

        updateInstances(entity, qEntity, engine->deltaTime.deltaTime);
        moveCamera(&engine->window, engine->window.window, &renderPass[0]->camera, engine->deltaTime.deltaTime);
        moveCamera(&engine->window, engine->window.window, &renderPass[1]->camera, engine->deltaTime.deltaTime);
        moveCamera(&engine->window, engine->window.window, &renderPass[2]->camera, engine->deltaTime.deltaTime);
        moveCamera(&engine->window, engine->window.window, &renderPass[3]->camera, engine->deltaTime.deltaTime);

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
