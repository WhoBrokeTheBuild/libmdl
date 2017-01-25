#ifndef MDL_LOADER_H
#define MDL_LOADER_H

#include <stdbool.h>

typedef struct mdl_model mdl_model_t;

typedef struct mdl_loader
{
    const char *ext;
    bool (*func)(mdl_model_t *, const char *, const char *);

} mdl_loader_t;

#endif // MDL_LOADER_H
