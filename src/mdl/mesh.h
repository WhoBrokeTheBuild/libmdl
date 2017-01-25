#ifndef MDL_MESH_H
#define MDL_MESH_H

typedef struct mdl_material mdl_material_t;

typedef struct mdl_mesh
{
    char *name;
    unsigned int count;
    float *verts;
    float *norms;
    float *txcds;
    mdl_material_t *mat;

} mdl_mesh_t;

void mdl_mesh_init(mdl_mesh_t *this);
void mdl_mesh_term(mdl_mesh_t *this);

void mdl_mesh_generate_cube(mdl_mesh_t *this, float size);
void mdl_mesh_generate_sphere(mdl_mesh_t *this, float radius, int slices, int stacks);

#endif // MDL_MESH_H
