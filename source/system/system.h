#include <stdint.h>

struct variables {
    double th;
    double dth;
};
struct params {
    double mass;
    double length;
};

typedef void (functionType)(uint32_t n, struct variables [n], struct params [n], struct variables [n]);
typedef void (methodType)(uint32_t n, struct variables [n], struct params [n], double time, functionType *);

struct method {
    char name[50];
    float coords[4];
    float color[4];

    methodType (*method);
};

struct system {
    float time;
    float pos[3];
    float tilt[2];

    int qMethod;
    struct method *method;

    int pendulumCount;
    int nodeCount;

    struct variables *var;
    struct params *params;
};

void freeSystem(void *pPtr);

methodType euler;
methodType heun;
methodType modified_euler;
methodType rk4;
methodType rk5;
methodType x20rk5;
