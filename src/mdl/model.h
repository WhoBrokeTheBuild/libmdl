#ifndef MDL_MODEL_H
#define MDL_MODEL_H

#include <stdbool.h>

typedef struct mdl_mesh mdl_mesh_t;

typedef struct mdl_model
{
    unsigned int count;
    mdl_mesh_t *meshes;

} mdl_model_t;

void mdl_model_init(mdl_model_t *this);
void mdl_model_term(mdl_model_t *this);

bool mdl_model_load_from_file(mdl_model_t *this, const char *filename, const char *name);

#endif // MDL_MODEL_H
