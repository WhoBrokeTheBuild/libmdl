#ifndef MDL_MATERIAL_H
#define MDL_MATERIAL_H

typedef struct mdl_material
{
    char *name;
    float ambient[4];
    float diffuse[4];
    float specular[4];
    float shininess;
    float dissolve;
    char *ambient_map;
    char *diffuse_map;
    char *specular_map;
    char *bump_map;
    char *refl_map;

} mdl_material_t;

void mdl_material_init(mdl_material_t *this);
void mdl_material_term(mdl_material_t *this);

void mdl_material_copy(mdl_material_t *this, mdl_material_t *other);

#endif // MDL_MATERIAL_H
