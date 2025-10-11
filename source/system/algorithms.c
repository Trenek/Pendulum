#include "system.h"

void euler(uint32_t N, struct variables var[N], struct params params[N], double t, functionType *fun) {
    constexpr uint32_t structSize = sizeof(struct variables) / sizeof(double);

    double (*const varD)[structSize] = (void *)var;
    double k1[N][structSize];

    fun(N, var, params, (void *)k1);

    for (uint32_t i = 0; i < N; i += 1) {
        for (uint32_t j = 0; j < structSize; j += 1) {
            varD[i][j] += t * k1[i][j];
        }
    }
}

void heun(uint32_t N, struct variables var[N], struct params params[N], double t, functionType *fun) {
    constexpr uint32_t structSize = sizeof(struct variables) / sizeof(double);

    double (*const varD)[structSize] = (void *)var;
    double temp[N][structSize];
    double k1[N][structSize];
    double k2[N][structSize];

    fun(N, var, params, (void *)k1);

    for (uint32_t i = 0; i < N; i += 1) {
        for (uint32_t j = 0; j < structSize; j += 1) {
            temp[i][j] = varD[i][j] + t * k1[i][j];
        }
    }

    fun(N, (void *)temp, params, (void *)k2);

    for (uint32_t i = 0; i < N; i += 1) {
        for (uint32_t j = 0; j < structSize; j += 1) {
            varD[i][j] += (t / 2) * (k1[i][j] + k2[i][j]);
        }
    }
}

void modified_euler(uint32_t N, struct variables var[N], struct params params[N], double t, functionType *fun) {
    constexpr uint32_t structSize = sizeof(struct variables) / sizeof(double);

    double (*const varD)[structSize] = (void *)var;
    double temp[N][structSize];
    double k1[N][structSize];
    double k2[N][structSize];

    fun(N, var, params, (void *)k1);

    for (uint32_t i = 0; i < N; i += 1) {
        for (uint32_t j = 0; j < structSize; j += 1) {
            temp[i][j] = varD[i][j] + t / 2 * k1[i][j];
        }
    }

    fun(N, (void *)temp, params, (void *)k2);

    for (uint32_t i = 0; i < N; i += 1) {
        for (uint32_t j = 0; j < structSize; j += 1) {
            varD[i][j] += t * k2[i][j];
        }
    }
}

void multArr(int N, double arr[N], double val) {
    for (int i = 0; i < N; i += 1) {
        arr[i] *= val;
    }
}

void rk4(uint32_t N, struct variables var[N], struct params params[N], double t, functionType *fun) {
    constexpr uint32_t structSize = sizeof(struct variables) / sizeof(double);

    double (*const varD)[structSize] = (void *)var;
    double temp[N][structSize];
    double k1[N][structSize];
    double k2[N][structSize];
    double k3[N][structSize];
    double k4[N][structSize];

    fun(N, var, params, (void *)k1);
    multArr(N * structSize, (void *)k1, t);

    for (uint32_t i = 0; i < N; i += 1) {
        for (uint32_t j = 0; j < structSize; j += 1) {
            temp[i][j] = varD[i][j] + k1[i][j] / 2;
        }
    }

    fun(N, (void *)temp, params, (void *)k2);
    multArr(N * structSize, (void *)k2, t);

    for (uint32_t i = 0; i < N; i += 1) {
        for (uint32_t j = 0; j < structSize; j += 1) {
            temp[i][j] = varD[i][j] + k2[i][j] / 2;
        }
    }

    fun(N, (void *)temp, params, (void *)k3);
    multArr(N * structSize, (void *)k3, t);

    for (uint32_t i = 0; i < N; i += 1) {
        for (uint32_t j = 0; j < structSize; j += 1) {
            temp[i][j] = varD[i][j] + k3[i][j];
        }
    }

    fun(N, (void *)temp, params, (void *)k4);
    multArr(N * structSize, (void *)k4, t);

    for (uint32_t i = 0; i < N; i += 1) {
        for (uint32_t j = 0; j < structSize; j += 1) {
            varD[i][j] += (k1[i][j] + 2 * k2[i][j] + 2 * k3[i][j] + k4[i][j]) / 6;
        }
    }
}

void rk5(uint32_t N, struct variables var[N], struct params params[N], double t, functionType *fun) {
    constexpr uint32_t structSize = sizeof(struct variables) / sizeof(double);

    double (*const varD)[structSize] = (void *)var;
    double temp[N][structSize];
    double k1[N][structSize];
    double k2[N][structSize];
    double k3[N][structSize];
    double k4[N][structSize];
    double k5[N][structSize];
    double k6[N][structSize];

    fun(N, var, params, (void *)k1);
    multArr(N * structSize, (void *)k1, t);

    for (uint32_t i = 0; i < N; i += 1) {
        for (uint32_t j = 0; j < structSize; j += 1) {
            temp[i][j] = varD[i][j] + k1[i][j] / 4;
        }
    }

    fun(N, (void *)temp, params, (void *)k2);
    multArr(N * structSize, (void *)k2, t);

    for (uint32_t i = 0; i < N; i += 1) {
        for (uint32_t j = 0; j < structSize; j += 1) {
            temp[i][j] = varD[i][j] + (k1[i][j] + k2[i][j]) / 8;
        }
    }

    fun(N, (void *)temp, params, (void *)k3);
    multArr(N * structSize, (void *)k3, t);

    for (uint32_t i = 0; i < N; i += 1) {
        for (uint32_t j = 0; j < structSize; j += 1) {
            temp[i][j] = varD[i][j] - k2[i][j] / 2 + k3[i][j];
        }
    }

    fun(N, (void *)temp, params, (void *)k4);
    multArr(N * structSize, (void *)k4, t);

    for (uint32_t i = 0; i < N; i += 1) {
        for (uint32_t j = 0; j < structSize; j += 1) {
            temp[i][j] = varD[i][j] + (3 * k1[i][j] + 9 * k4[i][j]) / 16;
        }
    }

    fun(N, (void *)temp, params, (void *)k5);
    multArr(N * structSize, (void *)k5, t);

    for (uint32_t i = 0; i < N; i += 1) {
        for (uint32_t j = 0; j < structSize; j += 1) {
            temp[i][j] = varD[i][j] + (- 3 * k1[i][j] + 2 * k2[i][j] + 12 * k3[i][j] - 12 * k4[i][j] + 8 * k5[i][j]) / 7;
        }
    }

    fun(N, (void *)temp, params, (void *)k6);
    multArr(N * structSize, (void *)k6, t);

    for (uint32_t i = 0; i < N; i += 1) {
        for (uint32_t j = 0; j < structSize; j += 1) {
            varD[i][j] += (7 * k1[i][j] + 32 * k3[i][j] + 12 * k4[i][j] + 32 * k5[i][j] + 7 * k6[i][j]) / 90;
        }
    }
}


void x20rk5(uint32_t N, struct variables var[N], struct params params[N], double t, functionType *fun) {
    int n = 20;
    for (int i = 0; i < n; i += 1) {
        rk5(N, var, params, t / n, fun);
    }
}
