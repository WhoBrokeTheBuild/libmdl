#include "model.h"
#include "mesh.h"
#include <stdio.h>
#include <stdlib.h>

void mdl_model_init(mdl_model_t *this)
{
    if (!this)
    {
        fprintf(stderr, "[Error]: (%s:%d) this is NULL\n", __FILE__, __LINE__);
        goto error;
    }

    this->count = 0;
    this->meshes = NULL;

error:;
}

void mdl_model_term(mdl_model_t *this)
{
    unsigned int i;

    if (!this)
    {
        fprintf(stderr, "[Error]: (%s:%d) this is NULL\n", __FILE__, __LINE__);
        goto error;
    }

    for (i = 0; i < this->count; ++i)
    {
        mdl_mesh_term(&this->meshes[i]);
    }
    free(this->meshes);

    mdl_model_init(this);

error:;
}
