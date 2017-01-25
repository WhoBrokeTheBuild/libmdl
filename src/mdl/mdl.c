#include "mdl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *mdl_strndup(const char *str, size_t maxlen)
{
    char *result;
    size_t len = strlen(str);

    if (maxlen < len)
    {
        len = maxlen;
    }

    result = (char *)malloc(len + 1);
    if (!result)
    {
        return NULL;
    }

    result[len] = '\0';
    return (char *)memcpy(result, str, len);
}

int mdl_getstr(char **lineptr, size_t *n, FILE *stream, char terminator, int offset)
{
    static const int MIN_CHUNK = 64;

    int nchars_avail;
    char *read_pos;
    int ret;

    if (!lineptr || !n || !stream)
    {
        return -1;
    }

    if (!*lineptr)
    {
        *n = MIN_CHUNK;
        *lineptr = malloc(*n);
        if (!*lineptr)
        {
            return -1;
        }
    }

    nchars_avail = *n - offset;
    read_pos = *lineptr + offset;

    for (;;)
    {
        register int c = getc(stream);

        //assert((*lineptr + *n) == (read_pos + nchars_avail));
        if (nchars_avail < 2)
        {
            if (*n > MIN_CHUNK)
                *n *= 2;
            else
                *n += MIN_CHUNK;

            nchars_avail = *n + *lineptr - read_pos;
            *lineptr = realloc(*lineptr, *n);
            if (!*lineptr)
            {
                return -1;
            }
            read_pos = *n - nchars_avail + *lineptr;
            //assert((*lineptr + *n) == (read_pos + nchars_avail));
        }

        if (ferror(stream))
        {
            return -1;
        }

        if (c == EOF)
        {
            if (read_pos == *lineptr)
                return -1;
            else
                break;
        }

        *read_pos++ = c;
        nchars_avail--;

        if (c == terminator)
            break;
    }

    *read_pos = '\0';

    ret = read_pos - (*lineptr + offset);
    return ret;
}

int mdl_getline(char **lineptr, size_t *n, FILE *stream)
{
    return mdl_getstr(lineptr, n, stream, '\n', 0);
}

unsigned int mdl_countchr(const char *str, char c)
{
    int i;
    unsigned int count = 0;

    for (i = 0; str[i]; ++i)
    {
        if (str[i] == c)
        {
            ++count;
        }
    }

    return count;
}

bool mdl_load_from_file(mdl_model_t *result, const char *filename, const char *name)
{
    int i = 0;
    char *pch = NULL;

    static mdl_loader_t model_loaders[] = {
        { ".obj", &mdl_model_load_from_obj },
        { ".fbx", &mdl_model_load_from_fbx },
        { NULL, NULL }
    };

    if (!result)
    {
        fprintf(stderr, "[Error]: (%s:%d) result is NULL\n", __FILE__, __LINE__);
        goto error;
    }

    if (!filename)
    {
        fprintf(stderr, "[Error]: (%s:%d) filename is NULL\n", __FILE__, __LINE__);
        goto error;
    }

    pch = strrchr(filename, '.');
    if (!pch)
    {
        fprintf(stderr, "[Error]: (%s:%d) No file extension found for '%s'\n", __FILE__, __LINE__, filename);
        goto error;
    }

    for (i = 0; model_loaders[i].ext; ++i)
    {
        if (strcmp(pch, model_loaders[i].ext) == 0)
        {
            return (*model_loaders[i].func)(result, filename, name);
        }
    }

    fprintf(stderr, "[Error]: (%s:%d) Unable to find model loader for '%s'\n", __FILE__, __LINE__, filename);

error:

    return false;
}
