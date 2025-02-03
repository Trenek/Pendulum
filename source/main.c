#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "VulkanTools.h"

struct node {
    double mass;
    double length;
    double acc;
};

void game(struct VulkanTools *vulkan, int M, int N, struct node params[M][N], double y[M][N][2]);

int main(void) {
    struct VulkanTools vulkan = setup();

    srand(time(NULL));
    do {
        FILE *file = fopen("examples/input.txt", "r");

        int pendulumCount = 0;
        int nodeCount = 0;

        fscanf(file, "%d %d", &pendulumCount, &nodeCount);

        struct node node[pendulumCount][nodeCount];
        double y[pendulumCount][nodeCount][2];

        for (int i = 0; i < pendulumCount; i += 1) {
            for (int j = 0; j < nodeCount; j += 1) {
                fscanf(file, "%lf %lf %lf %lf", &y[i][j][0], &y[i][j][1], &node[i][j].mass, &node[i][j].length);
                node[i][j].acc = 0;
            }
        }

        fclose(file);

        game(&vulkan, pendulumCount, nodeCount, node, y);
    } while (!glfwWindowShouldClose(vulkan.window));

    cleanup(vulkan);

    return 0;
}
