#include "loader_obj.h"
#include "mdl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const int OBJ_DEF_ARR_SIZE = 10;

int mdl_material_load_from_mtl(mdl_material_t **materials, const char *dir, const char *filename, int count)
{
    char path[MDL_MAX_PATH_LEN];
    FILE *fp = NULL;
    char *line = NULL;
    size_t len = 0;
    ssize_t read = 0;
    mdl_material_t *mat = NULL;

    if (dir)
    {
        strcpy(path, dir);
    }

    strcpy(path + strlen(dir), filename);

    fp = fopen(path, "r");
    if (!fp)
    {
        fprintf(stderr, "[Error]: (%s:%d) Failed to open %s\n", __FILE__, __LINE__, path);
    }

    while ((read = mdl_getline(&line, &len, fp)) != -1)
    {
        if (read == 0)
            break;
        if (line[0] == '#' || line[0] == '\n')
            continue;

        line[read - 1] = '\0';

        if (mat)
        {
            if (strncmp(line, "Kd", 2) == 0)
            {
                sscanf(line, "%*s %f %f %f", &mat->diffuse[0], &mat->diffuse[1], &mat->diffuse[2]);
            }
            else if (strncmp(line, "Ka", 2) == 0)
            {
                sscanf(line, "%*s %f %f %f", &mat->ambient[0], &mat->ambient[1], &mat->ambient[2]);
            }
            else if (strncmp(line, "Ks", 2) == 0)
            {
                sscanf(line, "%*s %f %f %f", &mat->specular[0], &mat->specular[1], &mat->specular[2]);
            }
            else if (strncmp(line, "Ns", 2) == 0)
            {
                sscanf(line, "%*s %f", &mat->shininess);
            }
            else if (strncmp(line, "map_Ka", 6) == 0)
            {
                if (dir)
                {
                    strcpy(path, dir);
                }
                strcpy(path + strlen(dir), line + 7);
                mat->ambient_map = mdl_strndup(path, MDL_MAX_PATH_LEN);
                if (!mat->ambient_map)
                {
                    fprintf(stderr, "[Error]: (%s:%d) Out Of Memory\n", __FILE__, __LINE__);
                    goto error;
                }
            }
            else if (strncmp(line, "map_Kd", 6) == 0)
            {
                if (dir)
                {
                    strcpy(path, dir);
                }
                strcpy(path + strlen(dir), line + 7);
                mat->diffuse_map = mdl_strndup(path, MDL_MAX_PATH_LEN);
                if (!mat->diffuse_map)
                {
                    fprintf(stderr, "[Error]: (%s:%d) Out Of Memory\n", __FILE__, __LINE__);
                    goto error;
                }
            }
            else if (strncmp(line, "map_Ks", 6) == 0)
            {
                if (dir)
                {
                    strcpy(path, dir);
                }
                strcpy(path + strlen(dir), line + 7);
                mat->specular_map = mdl_strndup(path, MDL_MAX_PATH_LEN);
                if (!mat->specular_map)
                {
                    fprintf(stderr, "[Error]: (%s:%d) Out Of Memory\n", __FILE__, __LINE__);
                    goto error;
                }
            }
            else if (strncmp(line, "bump", 4) == 0)
            {
                if (dir)
                {
                    strcpy(path, dir);
                }
                strcpy(path + strlen(dir), line + 5);
                mat->bump_map = mdl_strndup(path, MDL_MAX_PATH_LEN);
                if (!mat->bump_map)
                {
                    fprintf(stderr, "[Error]: (%s:%d) Out Of Memory\n", __FILE__, __LINE__);
                    goto error;
                }
            }
        }

        if (strncmp(line, "newmtl", 6) == 0)
        {
            ++count;
            *materials = realloc(*materials, count * sizeof(mdl_material_t));
            if (!*materials)
            {
                fprintf(stderr, "[Error]: (%s:%d) Out Of Memory\n", __FILE__, __LINE__);
                goto error;
            }
            mat = &(*materials)[count - 1];
            mdl_material_init(mat);

            mat->name = mdl_strndup(line + 7, MDL_MAX_NAME_LEN);
            if (!mat->name)
            {
                fprintf(stderr, "[Error]: (%s:%d) Out Of Memory\n", __FILE__, __LINE__);
                goto error;
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
    int i, j;
    FILE *fp = NULL;
    char *line = NULL;
    size_t len = 0;
    ssize_t read = 0;
    char *dir = NULL;
    char *pch = NULL;
    float tmp[3];

    int all_verts_cap = OBJ_DEF_ARR_SIZE;
    int all_norms_cap = OBJ_DEF_ARR_SIZE;
    int all_txcds_cap = OBJ_DEF_ARR_SIZE;
    float *all_verts = NULL;
    float *all_norms = NULL;
    float *all_txcds = NULL;
    int all_verts_index = 0;
    int all_norms_index = 0;
    int all_txcds_index = 0;

    int verts_loaded = 0;
    int norms_loaded = 0;
    int txcds_loaded = 0;

    unsigned int slash_count = 0;
    bool has_norm = false;
    bool has_txcd = false;
    int face[3][3];
    memset(face, 0, sizeof(face));

    mdl_mesh_t *mesh = NULL;
    int mesh_verts_cap = 10;
    int mesh_norms_cap = 10;
    int mesh_txcds_cap = 10;
    int mesh_verts_index = 0;
    int mesh_norms_index = 0;
    int mesh_txcds_index = 0;
    char mesh_name[MDL_MAX_NAME_LEN + 1];
    mesh_name[MDL_MAX_NAME_LEN] = '\0';
    bool read_first_mesh = false;

    int material_count = 0;
    mdl_material_t *materials = NULL;

    all_verts = malloc(sizeof(float) * all_verts_cap * 3);
    all_norms = malloc(sizeof(float) * all_norms_cap * 3);
    all_txcds = malloc(sizeof(float) * all_txcds_cap * 2);
    if (!all_verts || !all_norms || !all_txcds)
    {
        fprintf(stderr, "[Error]: (%s:%d) Out Of Memory\n", __FILE__, __LINE__);
        goto error;
    }

    mdl_model_init(this);
    ++this->count;
    this->meshes = malloc(sizeof(mdl_mesh_t) * this->count);
    if (!this->meshes)
    {
        fprintf(stderr, "[Error]: (%s:%d) Out Of Memory\n", __FILE__, __LINE__);
        goto error;
    }
    mesh = &this->meshes[0];
    mdl_mesh_init(mesh);

    mesh->verts = malloc(sizeof(float) * mesh_verts_cap * 3);
    mesh->norms = malloc(sizeof(float) * mesh_norms_cap * 3);
    mesh->txcds = malloc(sizeof(float) * mesh_txcds_cap * 2);
    if (!mesh->verts || !mesh->norms || !mesh->txcds)
    {
        fprintf(stderr, "[Error]: (%s:%d) Out Of Memory\n", __FILE__, __LINE__);
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
        fprintf(stderr, "[Error]: (%s:%d) Failed to open '%s'\n", __FILE__, __LINE__, filename);
        goto error;
    }

    while ((read = mdl_getline(&line, &len, fp)) != -1)
    {
        if (read == 0)
            break;
        if (line[0] == '#' || line[0] == '\n')
            continue;

        line[read - 1] = '\0';

        if (line[0] == 'v')
        {
            if (line[1] == 'n')
            {
                sscanf(line, "%*s %f %f %f", &tmp[0], &tmp[1], &tmp[2]);
                memcpy(all_norms + (all_norms_index * 3), tmp, sizeof(float) * 3);
                ++all_norms_index;
            }
            else if (line[1] == 't')
            {
                sscanf(line, "%*s %f %f", &tmp[0], &tmp[1]);
                memcpy(all_txcds + (all_txcds_index * 2), tmp, sizeof(float) * 2);
                ++all_txcds_index;
            }
            else
            {
                sscanf(line, "%*s %f %f %f", &tmp[0], &tmp[1], &tmp[2]);
                memcpy(all_verts + (all_verts_index * 3), tmp, sizeof(float) * 3);
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
                sscanf(line, "%*s %d//%d %d//%d %d//%d", &face[0][0], &face[0][2], &face[1][0], &face[1][2], &face[2][0], &face[2][2]);
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
                        sscanf(line, "%*s %d/%d/%d %d/%d/%d %d/%d/%d", &face[0][0], &face[0][1], &face[0][2], &face[1][0], &face[1][1], &face[1][2], &face[2][0], &face[2][1], &face[2][2]);
                    }
                    else
                    {
                        has_txcd = true;
                        sscanf(line, "%*s %d/%d %d/%d %d/%d", &face[0][0], &face[0][2], &face[1][0], &face[1][2], &face[2][0], &face[2][2]);
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
                mesh->verts = realloc(mesh->verts, sizeof(float) * mesh_verts_cap * 3);
                if (!mesh->verts)
                {
                    fprintf(stderr, "[Error]: (%s:%d) Out Of Memory\n", __FILE__, __LINE__);
                    goto error;
                }
            }

            if (mesh_norms_index >= mesh_norms_cap - 6)
            {
                mesh_norms_cap *= 2;
                mesh->norms = realloc(mesh->norms, sizeof(float) * mesh_norms_cap * 3);
                if (!mesh->norms)
                {
                    fprintf(stderr, "[Error]: (%s:%d) Out Of Memory\n", __FILE__, __LINE__);
                    goto error;
                }
            }

            if (mesh_txcds_index >= mesh_txcds_cap - 6)
            {
                mesh_txcds_cap *= 2;
                mesh->txcds = realloc(mesh->txcds, sizeof(float) * mesh_txcds_cap * 2);
                if (!mesh->txcds)
                {
                    fprintf(stderr, "[Error]: (%s:%d) Out Of Memory\n", __FILE__, __LINE__);
                    goto error;
                }
            }
        }
        else if (line[0] == 'g' || line[0] == 'o')
        {
            if (read_first_mesh)
            {
                mesh->verts = realloc(mesh->verts, sizeof(float) * mesh_verts_index * 3);

                if (mesh_norms_index == 0)
                {
                    //mesh_norms_index = mesh_verts_index;
                    //mesh->norms = realloc(mesh->norms, sizeof(float) * mesh_norms_index * 3);
                    //if (!mesh->norms)
                    //{
                    //    fprintf(stderr, "[Error]: (%s:%d) Out Of Memory\n", __FILE__, __LINE__);
                    //    goto error;
                    //}
                    //for (j = 0; j < mesh_verts_index; ++j)
                    //{
                    //    //calc_normal(&mesh->norms[j * 3 + 0], &mesh->verts[j * 3 + 0], &mesh->verts[j * 3 + 1], &mesh->verts[j * 3 + 2]);
                    //    //memcpy(&mesh->norms[j * 3 + 1], &mesh->norms[j * 3], sizeof(float) * 3);
                    //    //memcpy(&mesh->norms[j * 3 + 2], &mesh->norms[j * 3], sizeof(float) * 3);
                    //}
                }
                else
                {
                    mesh->norms = realloc(mesh->norms, sizeof(float) * mesh_norms_index * 3);
                }

                if (mesh_txcds_index == 0)
                {
                    free(mesh->txcds);
                    mesh->txcds = 0;
                }
                {
                    mesh->txcds = realloc(mesh->txcds, sizeof(float) * mesh_txcds_index * 2);
                }

                verts_loaded += mesh_verts_index;
                norms_loaded += mesh_norms_index;
                txcds_loaded += mesh_txcds_index;

                ++this->count;
                this->meshes = realloc(this->meshes, sizeof(mdl_mesh_t) * this->count);
                if (!this->meshes)
                {
                    fprintf(stderr, "[Error]: (%s:%d) Out Of Memory\n", __FILE__, __LINE__);
                    goto error;
                }
                mesh = &this->meshes[this->count - 1];
                mdl_mesh_init(mesh);

                mesh_verts_index = 0;
                mesh_norms_index = 0;
                mesh_txcds_index = 0;
                mesh_verts_cap = OBJ_DEF_ARR_SIZE;
                mesh_norms_cap = OBJ_DEF_ARR_SIZE;
                mesh_txcds_cap = OBJ_DEF_ARR_SIZE;
                mesh->verts = malloc(sizeof(float) * mesh_verts_cap * 3);
                mesh->norms = malloc(sizeof(float) * mesh_norms_cap * 3);
                mesh->txcds = malloc(sizeof(float) * mesh_txcds_cap * 2);
                if (!mesh->verts || !mesh->norms || !mesh->txcds)
                {
                    fprintf(stderr, "[Error]: (%s:%d) Out Of Memory\n", __FILE__, __LINE__);
                    goto error;
                }
            }
            else
            {
                read_first_mesh = true;
            }

            sscanf(line, "%*s %s" MDL_MAX_NAME_LEN_FMT "s", mesh_name);
            mesh->name = mdl_strndup(mesh_name, MDL_MAX_NAME_LEN);
            if (!mesh->name)
            {
                fprintf(stderr, "[Error]: (%s:%d) Out Of Memory\n", __FILE__, __LINE__);
                goto error;
            }
        }
        else if (strncmp(line, "mtllib", 6) == 0)
        {
            material_count += mdl_material_load_from_mtl(&materials, dir, line + 7, material_count);
        }
        else if (strncmp(line, "usemtl", 6) == 0)
        {
            for (i = 0; i < material_count; ++i)
            {
                if (strcmp(materials[i].name, line + 7) == 0)
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
            all_verts = realloc(all_verts, sizeof(float) * all_verts_cap * 3);
            if (!all_verts)
            {
                fprintf(stderr, "[Error]: (%s:%d) Out Of Memory\n", __FILE__, __LINE__);
                goto error;
            }
        }

        if (all_norms_index >= all_norms_cap - 1)
        {
            all_norms_cap *= 2;
            all_norms = realloc(all_norms, sizeof(float) * all_norms_cap * 3);
            if (!all_norms)
            {
                fprintf(stderr, "[Error]: (%s:%d) Out Of Memory\n", __FILE__, __LINE__);
                goto error;
            }
        }

        if (all_txcds_index >= all_txcds_cap - 1)
        {
            all_txcds_cap *= 2;
            all_txcds = realloc(all_txcds, sizeof(float) * all_txcds_cap * 2);
            if (!all_txcds)
            {
                fprintf(stderr, "[Error]: (%s:%d) Out Of Memory\n", __FILE__, __LINE__);
                goto error;
            }
        }
    }

    mesh->verts = realloc(mesh->verts, sizeof(float) * mesh_verts_index * 3);

    if (mesh_norms_index == 0)
    {
        mesh_norms_index = mesh_verts_index;
        mesh->norms = realloc(mesh->norms, sizeof(float) * mesh_norms_index * 3);
        for (j = 0; j < mesh_verts_index; ++j)
        {
            //calc_normal(&mesh->norms[j * 3 + 0], &mesh->verts[j * 3 + 0], &mesh->verts[j * 3 + 1], &mesh->verts[j * 3 + 2]);
            //vec3f_copy(&mesh->norms[j * 3 + 1], &mesh->norms[j * 3]);
            //vec3f_copy(&mesh->norms[j * 3 + 2], &mesh->norms[j * 3]);
        }
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
    else
    {
        mesh->txcds = realloc(mesh->txcds, sizeof(float) * mesh_txcds_index * 2);
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
