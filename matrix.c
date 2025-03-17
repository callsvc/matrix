#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define CL_TARGET_OPENCL_VERSION 200
#include <CL/opencl.h>

typedef float f32_t;
typedef int i32_t;
typedef unsigned char u8_t;

void quit(const char *);

FILE *GenerateMatrixFile(size_t, const char *);

void ReadMatrixFile(f32_t *out, FILE *file, size_t *count);

cl_program BuildProgram(cl_context context);

cl_device_id GetDevice(cl_context context);

void WriteMatrix(const char *filename, const f32_t *values, size_t size, size_t stride) {
    FILE *file = fopen(filename, "w");
    fprintf(file, "Matrix: %s [\n", filename);
    for (size_t index = 0; index < size; index += stride) {
        for (size_t inner = 0; inner < stride; inner++)
            fprintf(file, "\t%02.f", values[index + inner]);
        fprintf(file, "\n");
    }
    fprintf(file, "]\n");
    fclose(file);
}

size_t layout = 0;

int main() {
    cl_platform_id platform;
    clGetPlatformIDs(1, &platform, NULL);
    if (!platform)
        return -1;

    cl_device_id device;
    cl_uint count;
    clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, &count);

    printf("Total number of GPUs: %u\n", count);
    if (!count)
        quit("No device found");
    cl_context context = clCreateContext(NULL, 1, &device, NULL, 0, NULL);

    const size_t scalar = 9;
    layout = scalar * scalar;

    size_t devSize = 256;
    cl_device_id defaultDevice = GetDevice(context);
    char *vendor = malloc(devSize);
    clGetDeviceInfo(defaultDevice, CL_DEVICE_VENDOR, 0, NULL, &devSize);
    clGetDeviceInfo(defaultDevice, CL_DEVICE_VENDOR, devSize, vendor, NULL);
    if (devSize)
        printf("Vendor: %s\n", vendor);
    free(vendor);

    f32_t *floats = malloc(layout * 3 * sizeof(f32_t));

    FILE *file = fopen("matrix.csv", "r");
    if (!file)
        file = GenerateMatrixFile(layout, "matrix.csv");

    if (!file)
        quit("matrix.csv is missing");
    size_t total;
    ReadMatrixFile(floats, file, &total);
    if (file)
        fclose(file);

    const f32_t *first = floats;
    const f32_t *second = &floats[layout];
    f32_t *result = &floats[layout * 2];

    f32_t *test = calloc(sizeof(f32_t), layout);

    assert(second - first == result - second);

    cl_program main = BuildProgram(context);
    cl_kernel kernel = clCreateKernel(main, "__main", NULL);

    cl_mem matrix[3];

    cl_int error;
    cl_command_queue queue = clCreateCommandQueueWithProperties(context, defaultDevice, 0, &error);
    if (error != CL_SUCCESS)
        quit("Could not create a Command Queue");

    const size_t size = layout * sizeof(f32_t);

    for (i32_t max = 0; max < 2; max++) {
        matrix[max] = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, size, (void *) (max ? first : second), &error);
        assert(error == CL_SUCCESS);
    }
    matrix[2] = clCreateBuffer(context, CL_MEM_WRITE_ONLY, size, NULL, NULL);
    clSetKernelArg(kernel, 0, sizeof(cl_mem), &matrix[0]);
    clSetKernelArg(kernel, 1, sizeof(cl_mem), &matrix[1]);
    clSetKernelArg(kernel, 2, sizeof(cl_mem), &matrix[2]);

    size_t locals[2] = {scalar, scalar};

    size_t globals[2] = {total, total};
    error = clEnqueueNDRangeKernel(queue, kernel, 2, NULL, globals, locals, 0, NULL, NULL);

    clFinish(queue);
    clEnqueueReadBuffer(queue, matrix[2], CL_TRUE, 0, size, test, 0, NULL, NULL);

    if (memcmp(test, result, size) != 0)
        fprintf(stderr, "Failed to multiply matrices\n");

    WriteMatrix("first.txt", first, layout, scalar);
    WriteMatrix("second.txt", second, layout, scalar);
    WriteMatrix("result.txt", result, layout, scalar);

    WriteMatrix("opencl.txt", test, layout, scalar);
    free(floats);
    free(test);

    for (size_t buffer = 0; buffer < 3; buffer++)
        clReleaseMemObject(matrix[buffer]);

    clReleaseCommandQueue(queue);
    clReleaseKernel(kernel);
    clReleaseProgram(main);
    clReleaseContext(context);
}
