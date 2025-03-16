#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define CL_TARGET_OPENCL_VERSION 200
#include <CL/opencl.h>

typedef float f32_t;
typedef int i32_t;
typedef unsigned char u8_t;

void quit(const char*);
void ReadMatrixFile(f32_t *out, FILE *file, i32_t *count);

cl_program BuildProgram(cl_context context);
cl_device_id GetDevice(cl_context context);

int main() {
    cl_platform_id platform;
    clGetPlatformIDs(1, &platform, NULL);

    cl_device_id device;
    cl_uint count;
    clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, &count);

    cl_context context = clCreateContext(NULL, 1, &device, NULL, 0, NULL);

    f32_t *floats = malloc(1024 * sizeof(f32_t));

    FILE *file = fopen("matrix.csv", "r");
    if (!file)
        quit("matrix.csv is missing");
    i32_t total;
    ReadMatrixFile(floats, file, &total);
    if (file)
        fclose(file);

    assert(total % 3 == 0);
    const f32_t *first = floats;
    const f32_t *second = &floats[sizeof(float) * (total / 3)];
    f32_t *result = &floats[sizeof(f32_t) * (i32_t)(total / 1.5)];

    f32_t *test = calloc(1, second - first);

    assert(second - first == result - second);

    cl_program main = BuildProgram(context);
    cl_kernel kernel = clCreateKernel(main, "__main", NULL);

    cl_mem matrix[3];

    cl_int error;
    cl_command_queue queue = clCreateCommandQueueWithProperties(context, GetDevice(context), 0, &error);
    if (error != CL_SUCCESS)
        quit("Could not create a Command Queue");

    const size_t size = total / 3 * sizeof(f32_t);
    for (i32_t mat = 0; mat < 2; mat++) {
        matrix[mat] = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, size, (void*)(mat ? first : second), &error);
        assert(error == CL_SUCCESS);
    }
    matrix[2] = clCreateBuffer(context, CL_MEM_WRITE_ONLY, size, NULL, NULL);
    clSetKernelArg(kernel, 0, sizeof(cl_mem), &matrix[0]);
    clSetKernelArg(kernel, 1, sizeof(cl_mem), &matrix[1]);
    clSetKernelArg(kernel, 2, sizeof(cl_mem), &matrix[2]);

    size_t locals[2] = {total / 3, total / 3};

    size_t globals[2] = {total, total};
    error = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, globals, locals, 0, NULL, NULL);

    clFinish(queue);
    clEnqueueReadBuffer(queue, matrix[2], CL_TRUE, 0, size, test, 0, NULL, NULL);

    if (memcmp(test, result, size) != 0)
        fprintf(stderr, "Vector multiplication failed");

    for (i32_t index = 0; index < size; index++)
        fprintf(stdout, "%f ", test[index]);
    fprintf(stdout, "\n");

    free(floats);

    for (i32_t mat = 0; mat < 3; mat++)
        clReleaseMemObject(matrix[mat]);

    clReleaseKernel(kernel);
    clReleaseProgram(main);
    clReleaseContext(context);
}
