#ifndef MDL_H
#define MDL_H

#define MDL_VERSION_STR "0.0.1"

#define MDL_MAX_NAME_LEN 512
#define MDL_MAX_NAME_LEN_FMT "512"
#define MDL_MAX_PATH_LEN 1024

#include "loader.h"
#include "material.h"
#include "mesh.h"
#include "model.h"

#include "loader_fbx.h"
#include "loader_obj.h"

#include <stdbool.h>
#include <stdio.h>
#include <sys/types.h>

char *mdl_strndup(const char *str, size_t maxlen);
int mdl_getline(char **lineptr, size_t *n, FILE *stream);
unsigned int mdl_countchr(const char *str, char c);

bool mdl_load_from_file(mdl_model_t *result, const char *filename, const char *name);

#endif // MDL_H
