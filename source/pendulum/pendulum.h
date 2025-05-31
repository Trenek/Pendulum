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
