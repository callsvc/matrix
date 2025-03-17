#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <assert.h>

#include <sys/stat.h>
typedef float f32_t;
typedef int i32_t;
typedef unsigned char u8_t;

void quit(const char *);

void *GetAllFileContent(FILE *file, size_t *size) {
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

const char *trim(const char *in) {
    while (isspace(*in))
        in++;
    return in;
}

extern size_t layout;

FILE *GenerateMatrixFile(const size_t size, const char *filename) {
    size_t count = size * 3;
    FILE *file = fopen(filename, "w");
    srand(time(NULL)); // NOLINT(*-msc51-cpp)
    fprintf(file, "sep=;\n");
    for (size_t index = 0; index < count; index++) {
        if (index == count / 3 || index == (size_t) ((double) count / 1.5))
            fputc('\n', file);
        fprintf(file, "%02.f; ", (f32_t) (rand() % 100)); // NOLINT(*-msc50-cpp)
    }
    fputc('\n', file);
    fclose(file);
    file = fopen(filename, "r");
    return file;
}

void ReadMatrixFile(f32_t *out, FILE *file, size_t *count) {
    size_t buffer = 0;
    char *line = GetAllFileContent(file, &buffer);
    char *source = line;
    if (strstr(line, "sep"))
        line = strstr(line, "\n") + 1;
    *count = 0;
    for (const char *tok = strtok(line, ";"); tok && *tok; tok = strtok(NULL, ";\n")) {
        const char *number = trim(tok);
        if (isalnum(*number) && (*out++ = strtof(number, NULL)) >= 0)
            (*count)++;
        if (*count > layout * 3)
            quit("Out of bounds");
    }
    assert(*count == layout * 3);
    /*
    out -= *count;

    for (size_t index = 0; index < layout * 3; index++)
    printf("%02.f ", out[index]);
    puts("");
    */
    free(source);
}
