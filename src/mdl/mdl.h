#ifndef MDL_H
#define MDL_H

#define MDL_VER_STRING "1.0.0"

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

// clang-format off

#ifdef DEBUG
#  define MDL_ERROR(M, ...) do { fprintf(stderr, "[ERROR] (%s:%d): " M "\n", __FILE__, __LINE__, ##__VA_ARGS__); } while(false)
#  define MDL_INFO(M, ...) do { fprintf(stdout, "[INFO] (%s:%d): " M "\n", __FILE__, __LINE__, ##__VA_ARGS__); } while(false)
#else // RELEASE
#  define MDL_ERROR(M, ...) do { } while(false)
#  define MDL_INFO(M, ...) do { } while(false)
#endif // DEBUG

// clang-format on

#endif // MDL_H
