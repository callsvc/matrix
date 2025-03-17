__kernel void __main(__global float *first, __global float *second, __global float *result) {
    int row = get_global_id(0);
    int col = get_global_id(1);

    int size = get_local_size(0);

    float count = 0.f;
    for (int index = 0; index < size; index++) {
        count += first[row * size + index] * second[size * index + col];
    }
    result[row * size + col] = count;
}
