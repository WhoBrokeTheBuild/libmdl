#ifndef MDL_LOADER_FBX_H
#define MDL_LOADER_FBX_H

typedef struct mdl_model mdl_model_t;

#include <stdbool.h>

bool mdl_model_load_from_fbx(mdl_model_t *this, const char *filename, const char *name);

#endif // MDL_LOADER_FBX_H
