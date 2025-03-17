#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#define CL_TARGET_OPENCL_VERSION 200
#include <CL/opencl.h>

void quit(const char*);

typedef float f32_t;
typedef int i32_t;
typedef unsigned char u8_t;

cl_program CompileProgram(cl_context context, const char *lines, size_t size);
void* GetAllFileContent(FILE* file, size_t *size);
cl_device_id GetDevice(cl_context context);

cl_program BuildProgram(cl_context context) {
    struct stat attrib;
    FILE *source = fopen("kernel.cl", "r");
    if (!source)
        quit("kernel.cl not found");
    fstat(fileno(source), &attrib);
    char modified[100];
    snprintf(modified, sizeof(modified), "%ld_kernel.bin", attrib.st_mtime);
    FILE *binary = fopen(modified, "r");

    size_t count, fmt;
    if (!binary) {
        const char* lines = GetAllFileContent(source, &count);
        cl_program result = CompileProgram(context, lines, count);
        clGetProgramInfo(result, CL_PROGRAM_BINARY_SIZES, sizeof(fmt), &fmt, NULL);
        if (count < fmt) {
            void* area = realloc((char*)lines, fmt);
            if (area) lines = area;
        }

        clGetProgramInfo(result, CL_PROGRAM_BINARIES, sizeof(lines), &lines, NULL);
        FILE *machine = fopen(modified, "w");
        fwrite(lines, 1, fmt, machine);

        fclose(machine);
        free((char*)lines);

        clReleaseProgram(result);
    }
    if (!binary)
        binary = fopen(modified, "r");
    const u8_t* binp = GetAllFileContent(binary, &count);

    cl_int loaded;
    cl_int error;

    cl_device_id device = GetDevice(context);
    cl_program result = clCreateProgramWithBinary(context, 1, &device, &count, &binp, &loaded, &error);
    error = clBuildProgram(result, 0, NULL, NULL, NULL, NULL);
    if (loaded != CL_SUCCESS || error != CL_SUCCESS)
        quit("Main program not loaded");
    free((u8_t*)binp);
    fclose(binary);

    return result;
}
