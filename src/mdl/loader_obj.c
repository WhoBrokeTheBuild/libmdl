#include "loader_obj.h"
#include "mdl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int mdl_material_load_from_mtl(mdl_material_t **materials, const char *dir, const char *filename, int count)
{
    const int _MAX_LINE_LEN = 1024;

    FILE* fp = NULL;
    char line[_MAX_LINE_LEN];
    size_t linelen = 0;
    char path[MDL_MAX_PATH_LEN];
    mdl_material_t* mat = NULL;
    size_t dirlen = strlen(dir);

    if (dir)
    {
        strncpy(path, dir, MDL_MAX_PATH_LEN);
        if (path[dirlen] != '/' && path[dirlen] != '\\')
        {
            path[dirlen++] = '/';
        }
    }

    strcpy(path + dirlen, filename);

    fp = fopen(path, "r");
    if (fp == NULL)
    {
        MDL_ERROR("Failed to open %s", path);
        goto error;
    }

    while (fgets(line, _MAX_LINE_LEN, fp) != NULL)
    {
        linelen = strlen(line);

        if (linelen == 0) continue;
        if (line[0] == '#' || line[0] == '\n') continue;

        line[linelen--] = '\0';

        if (strncmp(line, "newmtl", sizeof("newmtl") - 1) == 0)
        {
            *materials = (mdl_material_t*)realloc(*materials, ++count * sizeof(mdl_material_t));
            if (*materials == NULL)
            {
                MDL_ERROR("Out of Memory");
                goto error;
            }

            mat = &(*materials)[count - 1];
            mdl_material_init(mat);

            mat->name = mdl_strndup(line + sizeof("newmtl ") - 1, MDL_MAX_NAME_LEN);
            if (mat->name == NULL)
            {
                MDL_ERROR("Out of Memory");
                goto error;
            }
        }
        else
        {
            if (mat)
            {
                if (line[0] == 'K')
                {
                    if (line[1] == 'a') // Ka
                    {
                        sscanf(line, "%*s %f %f %f", &mat->diffuse[0], &mat->diffuse[1], &mat->diffuse[2]);
                    }
                    else if (line[1] == 'd') // Kd
                    {
                        sscanf(line, "%*s %f %f %f", &mat->ambient[0], &mat->ambient[1], &mat->ambient[2]);
                    }
                    else if (line[1] == 's') // Ks
                    {
                        sscanf(line, "%*s %f %f %f", &mat->specular[0], &mat->specular[1], &mat->specular[2]);
                    }
                }
                else if (strncmp(line, "Ns", sizeof("Ns") - 1) == 0)
                {
                    sscanf(line, "%*s %f", &mat->shininess);
                }
                if (strncmp(line, "map_", sizeof("map_") - 1) == 0)
                {
                    if (line[1] == 'a') // map_Ka
                    {
                        strcpy(path + dirlen, line + sizeof("map_Ka ") - 1);
                        mat->ambient_map = mdl_strndup(path, MDL_MAX_PATH_LEN);
                        if (mat->ambient_map == NULL)
                        {
                            MDL_ERROR("Out of Memory");
                            goto error;
                        }
                    }
                    else if (line[1] == 'd') // map_Kd
                    {
                        strcpy(path + dirlen, line + sizeof("map_Kd ") - 1);
                        mat->diffuse_map = mdl_strndup(path, MDL_MAX_PATH_LEN);
                        if (mat->diffuse_map == NULL)
                        {
                            MDL_ERROR("Out of Memory");
                            goto error;
                        }
                    }
                    else if (line[1] == 's') // map_Ks
                    {
                        strcpy(path + dirlen, line + sizeof("map_Ks ") - 1);
                        mat->specular_map = mdl_strndup(path, MDL_MAX_PATH_LEN);
                        if (mat->specular_map == NULL)
                        {
                            MDL_ERROR("Out of Memory");
                            goto error;
                        }
                    }
                }
                else if (strncmp(line, "bump", sizeof("bump") - 1) == 0)
                {
                    strcpy(path + dirlen, line + sizeof("bump ") - 1);
                    mat->bump_map = mdl_strndup(path, MDL_MAX_PATH_LEN);
                    if (mat->bump_map == NULL)
                    {
                        MDL_ERROR("Out of Memory");
                        goto error;
                    }
                }
            }
            else
            {
                MDL_ERROR("Malformed MTL File %s", path);
            }
        }
    }

    fclose(fp);

    return count;

error:

    fclose(fp);

    return count;
}

bool mdl_model_load_from_obj(mdl_model_t *this, const char *filename, const char *name)
{
    const int _DEF_ARR_SIZE = 100;
    const int _MAX_LINE_LEN = 1024;

    int i;
    FILE *fp = NULL;
    char line[_MAX_LINE_LEN];
    size_t linelen = 0;
    char *dir = NULL;
    char *pch = NULL;
    float vec[3];
    int face[3][3];

    int all_verts_cap = _DEF_ARR_SIZE;
    int all_norms_cap = _DEF_ARR_SIZE;
    int all_txcds_cap = _DEF_ARR_SIZE;
    int all_verts_index = 0;
    int all_norms_index = 0;
    int all_txcds_index = 0;
    float *all_verts = NULL;
    float *all_norms = NULL;
    float *all_txcds = NULL;

    int verts_loaded = 0;
    int norms_loaded = 0;
    int txcds_loaded = 0;

    bool has_norm = false;
    bool has_txcd = false;
    unsigned int slash_count = 0;

    mdl_mesh_t *mesh = NULL;
    int mesh_verts_cap = 10;
    int mesh_norms_cap = 10;
    int mesh_txcds_cap = 10;
    int mesh_verts_index = 0;
    int mesh_norms_index = 0;
    int mesh_txcds_index = 0;
    char mesh_name[MDL_MAX_NAME_LEN];
    bool read_first_mesh = false;

    int material_count = 0;
    mdl_material_t *materials = NULL;

    all_verts = (float*)malloc(sizeof(float) * all_verts_cap * 3);
    all_norms = (float*)malloc(sizeof(float) * all_norms_cap * 3);
    all_txcds = (float*)malloc(sizeof(float) * all_txcds_cap * 2);
    if (all_verts == NULL || all_norms == NULL || all_txcds == NULL)
    {
        MDL_ERROR("Out of Memory");
        goto error;
    }

    mdl_model_init(this);
    this->meshes = malloc(sizeof(mdl_mesh_t) * ++this->count);
    if (this->meshes == NULL)
    {
        MDL_ERROR("Out of Memory");
        goto error;
    }

    mesh = &this->meshes[0];
    mdl_mesh_init(mesh);

    mesh->verts = (float*)malloc(sizeof(float) * mesh_verts_cap * 3);
    mesh->norms = (float*)malloc(sizeof(float) * mesh_norms_cap * 3);
    mesh->txcds = (float*)malloc(sizeof(float) * mesh_txcds_cap * 2);
    if (mesh->verts == NULL || mesh->norms == NULL || mesh->txcds == NULL)
    {
        MDL_ERROR("Out of Memory");
        goto error;
    }

    pch = strrchr(filename, '/');
    if (pch)
    {
        dir = mdl_strndup(filename, pch - filename + 1);
    }

    fp = fopen(filename, "r");
    if (!fp)
    {
        MDL_ERROR("Failed to Open %s", filename);
        goto error;
    }

    while (fgets(line, _MAX_LINE_LEN, fp) != NULL)
    {
        linelen = strlen(line);

        if (linelen == 0) continue;
        if (line[0] == '#' || line[0] == '\n') continue;

        line[linelen--] = '\0';

        if (line[0] == 'v')
        {
            if (line[1] == 'n') // vn
            {
                sscanf(line, "%*s %f %f %f", &vec[0], &vec[1], &vec[2]);
                memcpy(all_norms + (all_norms_index * 3), vec, sizeof(float) * 3);
                ++all_norms_index;
            }
            else if (line[1] == 't') // vt
            {
                sscanf(line, "%*s %f %f", &vec[0], &vec[1]);
                memcpy(all_txcds + (all_txcds_index * 2), vec, sizeof(float) * 2);
                ++all_txcds_index;
            }
            else // v
            {
                sscanf(line, "%*s %f %f %f", &vec[0], &vec[1], &vec[2]);
                memcpy(all_verts + (all_verts_index * 3), vec, sizeof(float) * 3);
                ++all_verts_index;
            }
        }
        else if (line[0] == 'f')
        {
            has_norm = false;
            has_txcd = false;
            memset(face, 0, sizeof(face));
            pch = strstr(line, "//");
            if (pch)
            {
                has_norm = true;
                sscanf(line, "%*s %d//%d %d//%d %d//%d",
                    &face[0][0], &face[0][2],
                    &face[1][0], &face[1][2],
                    &face[2][0], &face[2][2]);
            }
            else
            {
                pch = strchr(line, '/');
                if (pch)
                {
                    slash_count = mdl_countchr(line, '/');
                    if (slash_count > 3)
                    {
                        has_norm = true;
                        has_txcd = true;
                        sscanf(line, "%*s %d/%d/%d %d/%d/%d %d/%d/%d",
                            &face[0][0], &face[0][1], &face[0][2],
                            &face[1][0], &face[1][1], &face[1][2],
                            &face[2][0], &face[2][1], &face[2][2]);
                    }
                    else
                    {
                        has_txcd = true;
                        sscanf(line, "%*s %d/%d %d/%d %d/%d",
                            &face[0][0], &face[0][2],
                            &face[1][0], &face[1][2],
                            &face[2][0], &face[2][2]);
                    }
                }
                else
                {
                    sscanf(line, "%*s %d %d %d", &face[0][0], &face[1][0], &face[2][0]);
                }
            }

            for (i = 0; i < 3; ++i)
            {
                if (face[i][0] < 0)
                {
                    face[i][0] += all_verts_index;
                }
                if (face[i][1] < 0)
                {
                    face[i][1] += all_txcds_index;
                }
                if (face[i][2] < 0)
                {
                    face[i][2] += all_norms_index;
                }

                memcpy(mesh->verts + (mesh_verts_index * 3), all_verts + ((face[i][0] - 1) * 3), sizeof(float) * 3);
                ++mesh_verts_index;

                if (has_norm)
                {
                    memcpy(mesh->norms + (mesh_norms_index * 3), all_norms + ((face[i][2] - 1) * 3), sizeof(float) * 3);
                    ++mesh_norms_index;
                }

                if (has_txcd)
                {
                    memcpy(mesh->txcds + (mesh_txcds_index * 2), all_txcds + ((face[i][1] - 1) * 2), sizeof(float) * 2);
                    ++mesh_txcds_index;
                }

                ++mesh->count;
            }

            if (mesh_verts_index >= mesh_verts_cap - 6)
            {
                mesh_verts_cap *= 2;
                mesh->verts = (float*)realloc(mesh->verts, sizeof(float) * mesh_verts_cap * 3);
                if (mesh->verts == NULL)
                {
                    MDL_ERROR("Out of Memory");
                    goto error;
                }
            }

            if (mesh_norms_index >= mesh_norms_cap - 6)
            {
                mesh_norms_cap *= 2;
                mesh->norms = (float*)realloc(mesh->norms, sizeof(float) * mesh_norms_cap * 3);
                if (mesh->norms == NULL)
                {
                    MDL_ERROR("Out of Memory");
                    goto error;
                }
            }

            if (mesh_txcds_index >= mesh_txcds_cap - 6)
            {
                mesh_txcds_cap *= 2;
                mesh->txcds = (float*)realloc(mesh->txcds, sizeof(float) * mesh_txcds_cap * 2);
                if (mesh->txcds == NULL)
                {
                    MDL_ERROR("Out of Memory");
                    goto error;
                }
            }
        }
        else if (line[0] == 'g' || line[0] == 'o')
        {
            if (read_first_mesh)
            {
                mesh->verts = (float*)realloc(mesh->verts, sizeof(float) * mesh_verts_index * 3);

                if (mesh_norms_index == 0)
                {
                    // TODO: Calculate Normals
                    free(mesh->norms);
                    mesh->norms = NULL;
                }
                else
                {
                    mesh->norms = realloc(mesh->norms, sizeof(float) * mesh_norms_index * 3);
                }

                if (mesh_txcds_index == 0)
                {
                    free(mesh->txcds);
                    mesh->txcds = NULL;
                }
                {
                    mesh->txcds = realloc(mesh->txcds, sizeof(float) * mesh_txcds_index * 2);
                }

                verts_loaded += mesh_verts_index;
                norms_loaded += mesh_norms_index;
                txcds_loaded += mesh_txcds_index;

                this->meshes = (mdl_mesh_t*)realloc(this->meshes, sizeof(mdl_mesh_t) * ++this->count);
                if (this->meshes == NULL)
                {
                    MDL_ERROR("Out of Memory");
                    goto error;
                }

                mesh = &this->meshes[this->count - 1];
                mdl_mesh_init(mesh);

                mesh_verts_index = 0;
                mesh_norms_index = 0;
                mesh_txcds_index = 0;
                mesh_verts_cap = _DEF_ARR_SIZE;
                mesh_norms_cap = _DEF_ARR_SIZE;
                mesh_txcds_cap = _DEF_ARR_SIZE;
                mesh->verts = (float*)malloc(sizeof(float) * mesh_verts_cap * 3);
                mesh->norms = (float*)malloc(sizeof(float) * mesh_norms_cap * 3);
                mesh->txcds = (float*)malloc(sizeof(float) * mesh_txcds_cap * 2);
                if (mesh->verts == NULL || mesh->norms == NULL || mesh->txcds == NULL)
                {
                    MDL_ERROR("Out of Memory");
                    goto error;
                }

                read_first_mesh = true;
            }

            sscanf(line, "%*s %s" MDL_MAX_NAME_LEN_FMT "s", mesh_name);
            mesh->name = mdl_strndup(mesh_name, MDL_MAX_NAME_LEN);
            if (mesh->name == NULL)
            {
                MDL_ERROR("Out of Memory");
                goto error;
            }
        }
        else if (strncmp(line, "mtllib", sizeof("mtllib") - 1) == 0)
        {
            material_count += mdl_material_load_from_mtl(&materials, dir, line + sizeof("mtllib ") - 1, material_count);
        }
        else if (strncmp(line, "usemtl", sizeof("usemtl") - 1) == 0)
        {
            for (i = 0; i < material_count; ++i)
            {
                if (strcmp(materials[i].name, line + sizeof("usemtl") - 1) == 0)
                {
                    mesh->mat = malloc(sizeof(mdl_material_t));
                    mdl_material_init(mesh->mat);
                    mdl_material_copy(mesh->mat, &materials[i]);
                    break;
                }
            }
        }

        if (all_verts_index >= all_verts_cap - 1)
        {
            all_verts_cap *= 2;
            all_verts = (float*)realloc(all_verts, sizeof(float) * all_verts_cap * 3);
            if (all_verts == NULL)
            {
                MDL_ERROR("Out of Memory");
                goto error;
            }
        }

        if (all_norms_index >= all_norms_cap - 1)
        {
            all_norms_cap *= 2;
            all_norms = (float*)realloc(all_norms, sizeof(float) * all_norms_cap * 3);
            if (all_norms == NULL)
            {
                MDL_ERROR("Out of Memory");
                goto error;
            }
        }

        if (all_txcds_index >= all_txcds_cap - 1)
        {
            all_txcds_cap *= 2;
            all_txcds = (float*)realloc(all_txcds, sizeof(float) * all_txcds_cap * 2);
            if (all_txcds == NULL)
            {
                MDL_ERROR("Out of Memory");
                goto error;
            }
        }
    }

    mesh->verts = (float*)realloc(mesh->verts, sizeof(float) * mesh_verts_index * 3);

    if (mesh_norms_index == 0)
    {
        // TODO: Calculate Normals
        free(mesh->norms);
        mesh->norms = NULL;
    }
    else
    {
        mesh->norms = (float*)realloc(mesh->norms, sizeof(float) * mesh_norms_index * 3);
    }

    if (mesh_txcds_index == 0)
    {
        free(mesh->txcds);
        mesh->txcds = NULL;
    }
    else
    {
        mesh->txcds = (float*)realloc(mesh->txcds, sizeof(float) * mesh_txcds_index * 2);
    }

    verts_loaded += mesh_verts_index;
    norms_loaded += mesh_norms_index;
    txcds_loaded += mesh_txcds_index;

    printf("Loaded %s: Verts %d, Norms %d, Tex Coords %d\n", filename, verts_loaded, norms_loaded, txcds_loaded);

    free(all_verts);
    free(all_norms);
    free(all_txcds);

    fclose(fp);
    free(line);

    return true;

error:

    free(all_verts);
    free(all_norms);
    free(all_txcds);

    fclose(fp);
    free(line);

    return false;
}
