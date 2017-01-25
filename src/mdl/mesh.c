#include "mesh.h"
#include "material.h"
#include <stdio.h>
#include <stdlib.h>

void mdl_mesh_init(mdl_mesh_t *this)
{
    if (!this)
    {
        fprintf(stderr, "[Error]: (%s:%d) this is NULL\n", __FILE__, __LINE__);
        goto error;
    }

    this->name = NULL;
    this->count = 0;
    this->verts = NULL;
    this->norms = NULL;
    this->txcds = NULL;
    this->mat = NULL;

error:;
}

void mdl_mesh_term(mdl_mesh_t *this)
{
    if (!this)
    {
        fprintf(stderr, "[Error]: (%s:%d) this is NULL\n", __FILE__, __LINE__);
        goto error;
    }

    free(this->name);
    free(this->verts);
    free(this->norms);
    free(this->txcds);
    if (this->mat)
    {
        mdl_material_term(this->mat);
    }
    free(this->mat);

    mdl_mesh_init(this);

error:;
}
