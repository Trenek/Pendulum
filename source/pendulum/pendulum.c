#include <stdlib.h>
#include <malloc.h>

#include "pendulum.h"

void freeSystem(void *pPtr) {
    struct system *p = pPtr;

    free(p->method);
    free(p->node);
    free(p);
}
