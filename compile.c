#include <assert.h>
#include <stdlib.h>
#include <CL/opencl.h>

void quit(const char*);
cl_device_id GetDevice(cl_context context);

cl_program CompileProgram(cl_context context, const char *lines, size_t size) {
    cl_int error;
    cl_program result = clCreateProgramWithSource(context, 1, &lines, &size, &error);
    assert(error == CL_SUCCESS);
    error = clBuildProgram(result, 0, NULL, NULL, NULL, NULL);

    cl_device_id device = GetDevice(context);

    if (error != CL_SUCCESS) {
        clGetProgramBuildInfo(result, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &size);
        char* log = malloc(size);
        clGetProgramBuildInfo(result, device, CL_PROGRAM_BUILD_LOG, size, log, NULL);
        quit(log);
    }

    return result;
}