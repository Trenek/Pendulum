#include <stdlib.h>
#include <malloc.h>

#include "system.h"

void freeSystem(void *pPtr) {
    struct system *p = pPtr;

    free(p->method);
    free(p->var);
    free(p->params);
    free(p);
}
