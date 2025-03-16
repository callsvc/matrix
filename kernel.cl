__kernel void __main(__global float *first, __global float *second, __global float *result) {
    int row = get_global_id(0);
    int line = get_global_id(1);

    int size = get_local_size(0);

    int count = 0;
    for (int index = 0; index < size; index++) {
        count += first[row * size + index] * second[size * index + line];
    }
    result[row * size + line] = count;
}