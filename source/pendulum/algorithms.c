#include "pendulum.h"

void euler(int N, struct node *init, double t, void (*fun)(struct node *, double (*)[2])) {
    double k1[N][2];

    fun(init, k1);

    for (int i = 0; i < N; i += 1) {
        init[i].angle += t * k1[i][0];
        init[i].angularVelocity += t * k1[i][1];
    }
}

void heun(int N, struct node *init, double t, void (*fun)(struct node *, double (*)[2])) {
    struct node temp[N];
    double k1[N][2];
    double k2[N][2];

    fun(init, k1);

    for (int i = 0; i < N; i += 1) {
        temp[i].angle = init[i].angle + t * k1[i][0];
        temp[i].angularVelocity += init[i].angularVelocity + t * k1[i][1];
        temp[i].length = init[i].length;
        temp[i].mass = init[i].mass;
    }

    fun(temp, k2);

    for (int i = 0; i < N; i += 1) {
        init[i].angle += (t / 2) * (k1[i][0] + k2[i][0]);
        init[i].angularVelocity += (t / 2) * (k1[i][1] + k2[i][1]);
    }
}

void modified_euler(int N, struct node *init, double t, void (*fun)(struct node *, double (*)[2])) {
    struct node temp[N];
    double k1[N][2];
    double k2[N][2];

    fun(init, k1);

    for (int i = 0; i < N; i += 1) {
        temp[i].angle = init[i].angle + t / 2 * k1[i][0];
        temp[i].angularVelocity = init[i].angularVelocity + t / 2 * k1[i][1];
        temp[i].length = init[i].length;
        temp[i].mass = init[i].mass;
    }

    fun(temp, k2);

    for (int i = 0; i < N; i += 1) {
        init[i].angle += t * k2[i][0];
        init[i].angularVelocity += t * k2[i][1];
    }
}

void rk4(int N, struct node *init, double t, void (*fun)(struct node *, double (*)[2])) {
    struct node temp[N];
    double k1[N][2];
    double k2[N][2];
    double k3[N][2];
    double k4[N][2];

    fun(init, k1);

    for (int i = 0; i < N; i += 1) {
        k1[i][0] *= t;
        k1[i][1] *= t;
        temp[i].angle = init[i].angle + k1[i][0] / 2;
        temp[i].angularVelocity = init[i].angularVelocity + k1[i][1] / 2;
        temp[i].length = init[i].length;
        temp[i].mass = init[i].mass;
    }

    fun(temp, k2);

    for (int i = 0; i < N; i += 1) {
        k2[i][0] *= t;
        k2[i][1] *= t;
        temp[i].angle = init[i].angle + k2[i][0] / 2;
        temp[i].angularVelocity = init[i].angularVelocity + k2[i][1] / 2;
    }

    fun(temp, k3);

    for (int i = 0; i < N; i += 1) {
        k3[i][0] *= t;
        k3[i][1] *= t;
        temp[i].angle = init[i].angle + k3[i][0];
        temp[i].angularVelocity = init[i].angularVelocity + k3[i][1];
    }

    fun(temp, k4);

    for (int i = 0; i < N; i += 1) {
        k4[i][0] *= t;
        k4[i][1] *= t;
        init[i].angle += (k1[i][0] + 2 * k2[i][0] + 2 * k3[i][0] + k4[i][0]) / 6;
        init[i].angularVelocity += (k1[i][1] + 2 * k2[i][1] + 2 * k3[i][1] + k4[i][1]) / 6;
    }
}
