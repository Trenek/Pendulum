struct node {
    double angle;
    double angularVelocity;
    double mass;
    double length;
};

struct system {
    int pendulumCount;
    int nodeCount;

    struct node *node;
};

void freeSystem(void *pPtr);

void euler(int N, struct node *init, double t, void (*fun)(struct node *init, double (*result)[2]));
void heun(int N, struct node *init, double t, void (*fun)(struct node *init, double (*result)[2]));
void modified_euler(int N, struct node *init, double t, void (*fun)(struct node *init, double (*result)[2]));
void rk4(int N, struct node *init, double t, void (*fun)(struct node *init, double (*result)[2]));
