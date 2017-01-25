#include "material.h"
#include "mdl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void mdl_material_init(mdl_material_t *this)
{
    int i;

    if (!this)
    {
        fprintf(stderr, "[Error]: (%s:%d) this is NULL\n", __FILE__, __LINE__);
        goto error;
    }

    this->name = NULL;
    for (i = 0; i < 3; ++i)
    {
        this->ambient[i] = 0.0f;
        this->diffuse[i] = 0.0f;
        this->specular[i] = 0.0f;
    }
    this->ambient[3] = 1.0f;
    this->diffuse[3] = 1.0f;
    this->specular[3] = 1.0f;
    this->shininess = 0.0f;
    this->dissolve = 0.0f;
    this->ambient_map = NULL;
    this->diffuse_map = NULL;
    this->specular_map = NULL;
    this->bump_map = NULL;
    this->refl_map = NULL;

error:;
}

void mdl_material_term(mdl_material_t *this)
{
    if (!this)
    {
        fprintf(stderr, "[Error]: (%s:%d) this is NULL\n", __FILE__, __LINE__);
        goto error;
    }

    free(this->name);
    free(this->ambient_map);
    free(this->diffuse_map);
    free(this->specular_map);
    free(this->bump_map);
    free(this->refl_map);

    mdl_material_init(this);

error:;
}

void mdl_material_copy(mdl_material_t *this, mdl_material_t *other)
{
    if (!this)
    {
        fprintf(stderr, "[Error]: (%s:%d) this is NULL\n", __FILE__, __LINE__);
        goto error;
    }

    if (!other)
    {
        fprintf(stderr, "[Error]: (%s:%d) other is NULL\n", __FILE__, __LINE__);
        goto error;
    }

    mdl_material_term(this);

    if (other->name)
    {
        this->name = mdl_strndup(other->name, MDL_MAX_NAME_LEN);
        if (!this->name)
        {
            fprintf(stderr, "[Error]: (%s:%d) Out Of Memory\n", __FILE__, __LINE__);
            goto error;
        }
    }
    memcpy(this->ambient, other->ambient, sizeof(this->ambient));
    memcpy(this->diffuse, other->diffuse, sizeof(this->diffuse));
    memcpy(this->specular, other->specular, sizeof(this->specular));
    this->dissolve = other->dissolve;
    this->shininess = other->shininess;
    if (other->ambient_map)
    {
        this->ambient_map = mdl_strndup(other->ambient_map, MDL_MAX_PATH_LEN);
        if (!this->ambient_map)
        {
            fprintf(stderr, "[Error]: (%s:%d) Out Of Memory\n", __FILE__, __LINE__);
            goto error;
        }
    }
    if (other->diffuse_map)
    {
        this->diffuse_map = mdl_strndup(other->diffuse_map, MDL_MAX_PATH_LEN);
        if (!this->diffuse_map)
        {
            fprintf(stderr, "[Error]: (%s:%d) Out Of Memory\n", __FILE__, __LINE__);
            goto error;
        }
    }
    if (other->specular_map)
    {
        this->specular_map = mdl_strndup(other->specular_map, MDL_MAX_PATH_LEN);
        if (!this->specular_map)
        {
            fprintf(stderr, "[Error]: (%s:%d) Out Of Memory\n", __FILE__, __LINE__);
            goto error;
        }
    }
    if (other->bump_map)
    {
        this->bump_map = mdl_strndup(other->bump_map, MDL_MAX_PATH_LEN);
        if (!this->bump_map)
        {
            fprintf(stderr, "[Error]: (%s:%d) Out Of Memory\n", __FILE__, __LINE__);
            goto error;
        }
    }
    if (other->refl_map)
    {
        this->refl_map = mdl_strndup(other->refl_map, MDL_MAX_PATH_LEN);
        if (!this->refl_map)
        {
            fprintf(stderr, "[Error]: (%s:%d) Out Of Memory\n", __FILE__, __LINE__);
            goto error;
        }
    }

error:;
}
