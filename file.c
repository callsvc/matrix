#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <sys/stat.h>
typedef float f32_t;
typedef int i32_t;
typedef unsigned char u8_t;

void* GetAllFileContent(FILE* file, size_t *size) {
    size_t local;
    if (size == NULL)
        size = &local;
    struct stat attrib;
    fstat(fileno(file), &attrib);
    *size = attrib.st_size;
    void *content = malloc(*size);
    fread(content, *size, 1, file);

    return content;
}

const char* trim(const char* in) {
    while (isspace(*in))
        in++;
    return in;
}

void ReadMatrixFile(f32_t *out, FILE *file, i32_t *count) {
    char line[1024];
    *count = 0;
    while (fgets(line, sizeof(line), file)) {
        for (const char *tok = strtok(line, ";"); tok && *tok; tok = strtok(NULL, ";\n"))
            if ((*out++ = strtof(trim(tok), NULL)) >= 0)
                (*count)++;
    }
}