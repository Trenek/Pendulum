struct node {
    double th;
    double dth;
    double mass;
    double length;
};

struct system {
    float time;
    float pos[3];
    float tilt[2];

    int pendulumCount;
    int nodeCount;

    struct node *node;
};

void freeSystem(void *pPtr);

void euler(int N, struct node *init, double t, void (*fun)(int, struct node *, double (*)[2]));
void heun(int N, struct node *init, double t, void (*fun)(int, struct node *, double (*)[2]));
void modified_euler(int N, struct node *init, double t, void (*fun)(int, struct node *, double (*)[2]));
void rk4(int N, struct node *init, double t, void (*fun)(int, struct node *, double (*)[2]));
void rk5(int N, struct node *init, double t, void (*fun)(int, struct node *, double (*)[2]));
