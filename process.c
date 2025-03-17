#include <stdlib.h>
#include <stdio.h>

#define CL_TARGET_OPENCL_VERSION 200
#include <CL/opencl.h>
void quit(const char *fmt) {
    fprintf(stderr, "%s\n", fmt);
    exit(-1);
}

cl_device_id GetDevice(cl_context context) {
    cl_device_id device;
    size_t size;
    clGetContextInfo(context, CL_CONTEXT_DEVICES, 0, NULL, &size);
    clGetContextInfo(context, CL_CONTEXT_DEVICES, size, &device, NULL);
    return device;
}
