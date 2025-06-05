#include "pendulum.h"

void euler(int N, struct node *init, double t, void (*fun)(int, struct node *, double (*)[2])) {
    double k1[N][2];

    fun(N, init, k1);

    for (int i = 0; i < N; i += 1) {
        init[i].th += t * k1[i][0];
        init[i].dth += t * k1[i][1];
    }
}

void heun(int N, struct node *init, double t, void (*fun)(int, struct node *, double (*)[2])) {
    struct node temp[N];
    double k1[N][2];
    double k2[N][2];

    fun(N, init, k1);

    for (int i = 0; i < N; i += 1) {
        temp[i].th = init[i].th + t * k1[i][0];
        temp[i].dth = init[i].dth + t * k1[i][1];
        temp[i].length = init[i].length;
        temp[i].mass = init[i].mass;
    }

    fun(N, temp, k2);

    for (int i = 0; i < N; i += 1) {
        init[i].th += (t / 2) * (k1[i][0] + k2[i][0]);
        init[i].dth += (t / 2) * (k1[i][1] + k2[i][1]);
    }
}

void modified_euler(int N, struct node *init, double t, void (*fun)(int, struct node *, double (*)[2])) {
    struct node temp[N];
    double k1[N][2];
    double k2[N][2];

    fun(N, init, k1);

    for (int i = 0; i < N; i += 1) {
        temp[i].th = init[i].th + t / 2 * k1[i][0];
        temp[i].dth = init[i].dth + t / 2 * k1[i][1];
        temp[i].length = init[i].length;
        temp[i].mass = init[i].mass;
    }

    fun(N, temp, k2);

    for (int i = 0; i < N; i += 1) {
        init[i].th += t * k2[i][0];
        init[i].dth += t * k2[i][1];
    }
}

void rk4(int N, struct node *init, double t, void (*fun)(int, struct node *, double (*)[2])) {
    struct node temp[N];
    double k1[N][2];
    double k2[N][2];
    double k3[N][2];
    double k4[N][2];

    fun(N, init, k1);

    for (int i = 0; i < N; i += 1) {
        k1[i][0] *= t;
        k1[i][1] *= t;
        temp[i].th = init[i].th + k1[i][0] / 2;
        temp[i].dth = init[i].dth + k1[i][1] / 2;
        temp[i].length = init[i].length;
        temp[i].mass = init[i].mass;
    }

    fun(N, temp, k2);

    for (int i = 0; i < N; i += 1) {
        k2[i][0] *= t;
        k2[i][1] *= t;
        temp[i].th = init[i].th + k2[i][0] / 2;
        temp[i].dth = init[i].dth + k2[i][1] / 2;
    }

    fun(N, temp, k3);

    for (int i = 0; i < N; i += 1) {
        k3[i][0] *= t;
        k3[i][1] *= t;
        temp[i].th = init[i].th + k3[i][0];
        temp[i].dth = init[i].dth + k3[i][1];
    }

    fun(N, temp, k4);

    for (int i = 0; i < N; i += 1) {
        k4[i][0] *= t;
        k4[i][1] *= t;
        init[i].th += (k1[i][0] + 2 * k2[i][0] + 2 * k3[i][0] + k4[i][0]) / 6;
        init[i].dth += (k1[i][1] + 2 * k2[i][1] + 2 * k3[i][1] + k4[i][1]) / 6;
    }
}

void rk5(int N, struct node *init, double t, void (*fun)(int, struct node *, double (*)[2])) {
    struct node temp[N];
    double k1[N][2];
    double k2[N][2];
    double k3[N][2];
    double k4[N][2];
    double k5[N][2];
    double k6[N][2];

    fun(N, init, k1);

    for (int i = 0; i < N; i += 1) {
        k1[i][0] *= t;
        k1[i][1] *= t;
        temp[i].th = init[i].th + k1[i][0] / 4;
        temp[i].dth = init[i].dth + k1[i][1] / 4;
        temp[i].length = init[i].length;
        temp[i].mass = init[i].mass;
    }

    fun(N, temp, k2);

    for (int i = 0; i < N; i += 1) {
        k2[i][0] *= t;
        k2[i][1] *= t;
        temp[i].th = init[i].th + k1[i][0] / 8 + k2[i][0] / 8;
        temp[i].dth = init[i].dth + k1[i][1] / 8 + k2[i][1] / 8;
    }

    fun(N, temp, k3);

    for (int i = 0; i < N; i += 1) {
        k3[i][0] *= t;
        k3[i][1] *= t;
        temp[i].th = init[i].th - k2[i][0] / 2 + k3[i][0];
        temp[i].dth = init[i].dth - k2[i][1] / 2 + k3[i][1];
    }

    fun(N, temp, k4);

    for (int i = 0; i < N; i += 1) {
        k4[i][0] *= t;
        k4[i][1] *= t;
        temp[i].th = init[i].th + 3 * k1[i][0] / 16 + 9 * k4[i][0] / 16;
        temp[i].dth = init[i].dth + 3 * k1[i][1] / 16 + 9 * k4[i][1] / 16;
    }

    fun(N, temp, k5);

    for (int i = 0; i < N; i += 1) {
        k5[i][0] *= t;
        k5[i][1] *= t;
        temp[i].th = init[i].th - 3 * k1[i][0] / 7 + 2 * k2[i][0] / 7 + 12 * k3[i][0] / 7 - 12 * k4[i][0] / 7 + 8 * k5[i][0] / 7;
        temp[i].dth = init[i].dth - 3 * k1[i][1] / 7 + 2 * k2[i][1] / 7 + 12 * k3[i][1] / 7 - 12 * k4[i][1] / 7 + 8 * k5[i][1] / 7;
    }

    fun(N, temp, k6);

    for (int i = 0; i < N; i += 1) {
        k6[i][0] *= t;
        k6[i][1] *= t;
        init[i].th += (7 * k1[i][0] + 32 * k3[i][0] + 12 * k4[i][0] + 32 * k5[i][0] + 7 * k6[i][0]) / 90;
        init[i].dth += (7 * k1[i][1] + 32 * k3[i][1] + 12 * k4[i][1] + 32 * k5[i][1] + 7 * k6[i][1]) / 90;
    }
}
