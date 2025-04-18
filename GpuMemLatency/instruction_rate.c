#include "opencltest.h"

float fp64_instruction_rate_test(cl_context context,
    cl_command_queue command_queue,
    uint32_t thread_count,
    uint32_t local_size,
    uint32_t chase_iterations,
    int float4_element_count,
    cl_mem a_mem_obj,
    cl_mem result_obj,
    cl_float* A,
    cl_float* result);

float fp16_instruction_rate_test(cl_context context,
    cl_command_queue command_queue,
    uint32_t thread_count,
    uint32_t local_size,
    uint32_t chase_iterations,
    int float4_element_count,
    cl_mem a_mem_obj,
    cl_mem result_obj,
    cl_float* A,
    cl_float* result);

float run_rate_test(cl_context context,
    cl_command_queue command_queue,
    cl_kernel kernel,
    uint32_t thread_count,
    uint32_t local_size,
    uint32_t chase_iterations,
    int float4_element_count,
    cl_mem a_mem_obj,
    cl_mem result_obj,
    cl_float* A,
    cl_float* result,
    float totalOps);

float run_latency_test(cl_context context,
    cl_command_queue command_queue,
    cl_kernel kernel,
    uint32_t chase_iterations,
    int float4_element_count,
    cl_mem a_mem_obj,
    cl_mem result_obj,
    cl_float* A,
    cl_float* result,
    float opsPerIteration);

float global_totalOps;

float instruction_rate_test(cl_context context,
    cl_command_queue command_queue,
    uint32_t thread_count,
    uint32_t local_size,
    uint32_t chase_iterations,
    int forcefp16,
    int forcefp64)
{
    size_t global_item_size = thread_count;
    size_t local_item_size = local_size;
    float gOpsPerSec = 0, opsPerIteration;
    cl_int ret;
    int64_t time_diff_ms;
    int float4_element_count = thread_count * 4;

    cl_program program = build_program(context, "instruction_rate_kernel.cl", NULL);
    if (saveprogram) write_program(program, "irate");
    cl_kernel int32_add_rate_kernel = clCreateKernel(program, "int32_add_rate_test", &ret);
    cl_kernel int32_mul_rate_kernel = clCreateKernel(program, "int32_mul_rate_test", &ret);
    cl_kernel fp32_add_rate_kernel = clCreateKernel(program, "fp32_add_rate_test", &ret);
    cl_kernel fp32_fma_rate_kernel = clCreateKernel(program, "fp32_fma_rate_test", &ret);
    cl_kernel fp32_builtin_fma_rate_kernel = clCreateKernel(program, "fp32_builtin_fma_rate_test", &ret);
    cl_kernel fp32_mad_rate_kernel = clCreateKernel(program, "fp32_mad_rate_test", &ret);
    cl_kernel fp32_rcp_rate_kernel = clCreateKernel(program, "fp32_rcp_rate_test", &ret);
    cl_kernel fp32_rsqrt_rate_kernel = clCreateKernel(program, "fp32_rsqrt_rate_test", &ret);
    cl_kernel mix_fp32_int32_add_rate_kernel = clCreateKernel(program, "mix_fp32_int32_add_rate_test", &ret);
    cl_kernel mix_fp32_int32_addmul_rate_kernel = clCreateKernel(program, "mix_fp32_int32_addmul_rate_test", &ret);
    cl_kernel int64_add_rate_kernel = clCreateKernel(program, "int64_add_rate_test", &ret);
    cl_kernel int64_mul_rate_kernel = clCreateKernel(program, "int64_mul_rate_test", &ret);
    cl_kernel int16_add_rate_kernel = clCreateKernel(program, "int16_add_rate_test", &ret);
    cl_kernel int16_mul_rate_kernel = clCreateKernel(program, "int16_mul_rate_test", &ret);
    cl_kernel int8_add_rate_kernel = clCreateKernel(program, "int8_add_rate_test", &ret);
    cl_kernel int8_mul_rate_kernel = clCreateKernel(program, "int8_mul_rate_test", &ret);
    cl_kernel fp32_fma_latency_kernel = clCreateKernel(program, "fp32_fma_latency_test", &ret);
    cl_kernel fp32_add_latency_kernel = clCreateKernel(program, "fp32_add_latency_test", &ret);
    cl_kernel int32_add_latency_kernel = clCreateKernel(program, "int32_add_latency_test", &ret);
    cl_kernel int32_mul_latency_kernel = clCreateKernel(program, "int32_mul_latency_test", &ret);

    float* A = (float*)malloc(sizeof(float) * float4_element_count * 4);
    float* result = (float*)malloc(sizeof(float) * 4 * thread_count);

    if (!A || !result)
    {
        fprintf(stderr, "Failed to allocate memory instruction rate test\n");
    }

    cl_mem a_mem_obj = clCreateBuffer(context, CL_MEM_READ_ONLY, float4_element_count * sizeof(float), NULL, &ret);
    cl_mem result_obj = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(float) * 4 * thread_count, NULL, &ret);

    // Integer test first
    uint32_t *int32_A = (uint32_t*)A;
    for (int i = 0; i < float4_element_count * 4; i++)
    {
        int32_A[i] = i + 1;
    }

    // 4x int4 * 8 per iteration, and count the loop increment too
    opsPerIteration = 4.0f * 8.0f;
    float int32_add_rate = run_rate_test(context, command_queue, int32_add_rate_kernel, thread_count, local_size, chase_iterations,
        float4_element_count, a_mem_obj, result_obj, A, result, opsPerIteration);
    fprintf(stderr, "INT32 G Adds/sec: %f\n", int32_add_rate);

    float int32_add_latency = run_latency_test(context, command_queue, int32_add_latency_kernel, chase_iterations, float4_element_count, a_mem_obj, result_obj, A, result, 8.0f);
    fprintf(stderr, "INT32 add latency: %f ns\n", int32_add_latency);

    opsPerIteration = 4.0f * 8.0f;
    float int32_mul_rate = run_rate_test(context, command_queue, int32_mul_rate_kernel, thread_count, local_size, (chase_iterations / 2),
        float4_element_count, a_mem_obj, result_obj, A, result, opsPerIteration);
    fprintf(stderr, "INT32 G Multiplies/sec: %f\n", int32_mul_rate);

    float int32_mul_latency = run_latency_test(context, command_queue, int32_mul_latency_kernel, chase_iterations, float4_element_count, a_mem_obj, result_obj, A, result, 8.0f);
    fprintf(stderr, "INT32 mul latency: %f ns\n", int32_mul_latency);

    // FP32 add and fma test
    cl_float* fp32_A = (cl_float*)A;
    for (int i = 0; i < float4_element_count * 4; i++)
    {
        fp32_A[i] = 0.5f * i;
    }

    opsPerIteration = 4.0f * 8.0f;

    float fp32_add_rate = run_rate_test(context, command_queue, fp32_add_rate_kernel, thread_count, local_size, chase_iterations,
        float4_element_count, a_mem_obj, result_obj, A, result, opsPerIteration);
    fprintf(stderr, "FP32 G Adds/sec: %f\n", fp32_add_rate);

    float fp32_add_latency = run_latency_test(context, command_queue, fp32_add_latency_kernel, chase_iterations, float4_element_count, a_mem_obj, result_obj, A, result, 8.0f);
    fprintf(stderr, "FP32 add latency: %f ns\n", fp32_add_latency);

    float fp32_fma_rate = run_rate_test(context, command_queue, fp32_fma_rate_kernel, thread_count, local_size, chase_iterations,
        float4_element_count, a_mem_obj, result_obj, A, result, opsPerIteration);
    fprintf(stderr, "FP32 G FMA/sec: %f : %f GFLOPs\n", fp32_fma_rate, fp32_fma_rate * 2); 

    float builtin_fp32_fma_rate = run_rate_test(context, command_queue, fp32_builtin_fma_rate_kernel, thread_count, local_size, chase_iterations,
        float4_element_count, a_mem_obj, result_obj, A, result, opsPerIteration);
    fprintf(stderr, "FP32 G fma()/sec: %f : %f GFLOPs\n", builtin_fp32_fma_rate, builtin_fp32_fma_rate * 2);

    fp32_fma_rate = run_rate_test(context, command_queue, fp32_mad_rate_kernel, thread_count, local_size, chase_iterations,
        float4_element_count, a_mem_obj, result_obj, A, result, opsPerIteration);
    fprintf(stderr, "FP32 G mad()/sec: %f : %f GFLOPs\n", fp32_fma_rate, fp32_fma_rate * 2);

    float fp32_fma_latency = run_latency_test(context, command_queue, fp32_fma_latency_kernel, chase_iterations, float4_element_count, a_mem_obj, result_obj, A, result, 8.0f);
    fprintf(stderr, "FP32 FMA latency: %f ns\n", fp32_fma_latency);

    float fp32_rcp_rate = run_rate_test(context, command_queue, fp32_rcp_rate_kernel, thread_count, local_size, chase_iterations,
        float4_element_count, a_mem_obj, result_obj, A, result, opsPerIteration);
    fprintf(stderr, "FP32 G native_recip/sec: %f\n", fp32_rcp_rate);

    float fp32_rsqrt_rate = run_rate_test(context, command_queue, fp32_rsqrt_rate_kernel, thread_count, local_size, chase_iterations,
        float4_element_count, a_mem_obj, result_obj, A, result, opsPerIteration);
    fprintf(stderr, "FP32 G native_rsqrt/sec: %f\n", fp32_rsqrt_rate);

    // Mixed INT32 and FP32 - 4 FP32, 4 INT32, and the loop increment
    // takes FP inputs and converts some to int
    opsPerIteration = 4.0f * 8.0f + 1.0f;
    float mix_fp32_int32_rate = run_rate_test(context, command_queue, mix_fp32_int32_add_rate_kernel, thread_count, local_size, chase_iterations,
        float4_element_count, a_mem_obj, result_obj, A, result, opsPerIteration);
    fprintf(stderr, "Mixed INT32 and FP32 G Adds/sec: %f\n", mix_fp32_int32_rate);

    // Test the same with integer multiplies
    mix_fp32_int32_rate = run_rate_test(context, command_queue, mix_fp32_int32_addmul_rate_kernel, thread_count, local_size, chase_iterations,
        float4_element_count, a_mem_obj, result_obj, A, result, opsPerIteration);
    fprintf(stderr, "Mixed INT32 Multiplies and FP32 G Adds/sec: %f\n", mix_fp32_int32_rate);

    // INT64 add test
    cl_ulong* int64_A = (cl_ulong*)A;
    for (int i = 0; i < float4_element_count * 2; i++)
    {
        int64_A[i] = i * 2;
    }

    opsPerIteration = 2.0f * 8.0f;
    float int64_add_rate = run_rate_test(context, command_queue, int64_add_rate_kernel, thread_count, local_size, chase_iterations / 2,
        float4_element_count, a_mem_obj, result_obj, A, result, opsPerIteration);
    fprintf(stderr, "INT64 G Adds/sec: %f\n", int64_add_rate);

    opsPerIteration = 2.0f * 8.0f;
    float int64_mul_rate = run_rate_test(context, command_queue, int64_mul_rate_kernel, thread_count, local_size, chase_iterations / 8,
        float4_element_count, a_mem_obj, result_obj, A, result, opsPerIteration);
    fprintf(stderr, "INT64 G Multiplies/sec: %f\n", int64_mul_rate);

    // INT16 (short) tests
    cl_ushort* int16_A = (cl_ushort*)A;
    for (int i = 0; i < float4_element_count * 8; i++)
    {
        int16_A[i] = i;
    }

    // short8
    opsPerIteration = 8.0f * 8.0f;
    float int16_add_rate = run_rate_test(context, command_queue, int16_add_rate_kernel, thread_count, local_size, chase_iterations,
        float4_element_count, a_mem_obj, result_obj, A, result, opsPerIteration);
    fprintf(stderr, "INT16 G Adds/sec: %f\n", int16_add_rate);

    float int16_mul_rate = run_rate_test(context, command_queue, int16_mul_rate_kernel, thread_count, local_size, chase_iterations,
        float4_element_count, a_mem_obj, result_obj, A, result, opsPerIteration);
    fprintf(stderr, "INT16 G Multiplies/sec: %f \n", int16_mul_rate);

    // INT8 (char) tests
    cl_char* int8_A = (cl_char*)A;
    for (int i = 0; i < float4_element_count * 8; i++)
    {
        int8_A[i] = i;
    }

    uint32_t int8_chase_iterations = chase_iterations / 10;
    opsPerIteration = 16.0f * 8.0f;
    float int8_add_rate = run_rate_test(context, command_queue, int8_add_rate_kernel, thread_count, local_size, int8_chase_iterations,
        float4_element_count, a_mem_obj, result_obj, A, result, opsPerIteration);
    fprintf(stderr, "INT8 G Adds/sec: %f\n", int8_add_rate);

    float int8_mul_rate = run_rate_test(context, command_queue, int8_mul_rate_kernel, thread_count, local_size, int8_chase_iterations,
        float4_element_count, a_mem_obj, result_obj, A, result, opsPerIteration);
    fprintf(stderr, "INT8 G Multiplies/sec: %f\n", int8_mul_rate);

    short checkExtensionSupport(const char *extension_name);
    
    if (checkExtensionSupport("cl_khr_fp64") || forcefp64) {
        fp64_instruction_rate_test(context, command_queue, thread_count, local_size, chase_iterations, float4_element_count,
            a_mem_obj, result_obj, A, result);
    }
    else {
        fprintf(stderr, "FP64 not supported\n");
    }

    if (checkExtensionSupport("cl_khr_fp16") || forcefp16) {
        fp16_instruction_rate_test(context, command_queue, thread_count, local_size, chase_iterations, float4_element_count,
            a_mem_obj, result_obj, A, result);
    }
    else {
        fprintf(stderr, "FP16 not supported\n");
    }

cleanup:
    clFlush(command_queue);
    clFinish(command_queue);
    clReleaseMemObject(a_mem_obj);
    clReleaseMemObject(result_obj);
    free(A);
    free(result);
    return gOpsPerSec;
}

// Runs an instruction rate test. The kernel is expected to perform opsPerIteration * chase_iterations operations
// Mostly simplifies the uber instruction rate test above. Expects memory to be pre-allocated for example.
// Returns GOPS
float run_rate_test(cl_context context,
    cl_command_queue command_queue,
    cl_kernel kernel,
    uint32_t thread_count,
    uint32_t local_size,
    uint32_t chase_iterations,
    int float4_element_count,
    cl_mem a_mem_obj,
    cl_mem result_obj,
    cl_float* A,
    cl_float* result,
    float opsPerIteration)
{
    size_t global_item_size = thread_count;
    size_t local_item_size = local_size;
    cl_int ret;
    float totalOps, gOpsPerSec;
    uint64_t time_diff_ms = 0;

    memset(result, 0, sizeof(float) * 4 * thread_count);

    ret = clEnqueueWriteBuffer(command_queue, a_mem_obj, CL_TRUE, 0, float4_element_count * sizeof(float), A, 0, NULL, NULL);
    ret = clEnqueueWriteBuffer(command_queue, result_obj, CL_TRUE, 0, sizeof(float) * 4 * thread_count, result, 0, NULL, NULL);
    clSetKernelArg(kernel, 0, sizeof(cl_mem), (void*)&a_mem_obj);
    clSetKernelArg(kernel, 1, sizeof(cl_int), (void*)&chase_iterations);
    clSetKernelArg(kernel, 2, sizeof(cl_mem), (void*)&result_obj);
    clFinish(command_queue);

    //fprintf(stderr, "Submitting fp32 add kernel to command queue\n");
    // start with a low iteration count and try to make it work for all GPUs without needing manual iteration adjustment
    while (time_diff_ms < TARGET_TIME_MS / 2) {
        start_timing();
        ret = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &global_item_size, &local_item_size, 0, NULL, NULL);
        if (ret != CL_SUCCESS)
        {
            fprintf(stderr, "Failed to submit kernel to command queue. clEnqueueNDRangeKernel returned %d\n", ret);
            gOpsPerSec = 0;
            return 0;
        }

        ret = clFinish(command_queue);
        if (ret != CL_SUCCESS)
        {
            printf("Failed to finish command queue. clFinish returned %d\n", ret);
            gOpsPerSec = 0;
            return 0;
        }

        time_diff_ms = end_timing();

        totalOps = (float)chase_iterations * opsPerIteration * (float)thread_count;
        gOpsPerSec = ((float)totalOps / 1e9) / ((float)time_diff_ms / 1000);
        //fprintf(stderr, "chase iterations: %d, thread count: %d\n", chase_iterations, thread_count);
        //fprintf(stderr, "total ops: %f (%.2f G)\ntotal time: %llu ms\n", totalOps, totalOps / 1e9, time_diff_ms);

        chase_iterations = adjust_iterations(chase_iterations, time_diff_ms);
        clSetKernelArg(kernel, 1, sizeof(cl_int), (void*)&chase_iterations);
    }

    return gOpsPerSec;
}


// Variation of the test above but input array size is aligned with assumed wave size.
// if partitioning pattern, this will test partitioning with active waves in the specified pattern
float run_divergence_rate_test(cl_context context,
    cl_command_queue command_queue,
    uint32_t thread_count,
    uint32_t local_size,
    uint32_t wave,
    int *partitionPattern)
{
    size_t global_item_size = thread_count;
    size_t local_item_size = local_size;
    uint32_t active_threads = thread_count;
    cl_int ret;
    float totalOps, gOpsPerSec;
    uint64_t time_diff_ms = 0;
    uint32_t chase_iterations = 2500000;

    cl_program program = build_program(context, "instruction_rate_kernel.cl", NULL);
    cl_kernel kernel = clCreateKernel(program, partitionPattern == NULL ? "fp32_divergence_rate_test" : "fp32_partition_rate_test", &ret);
    
    float* result = (float*)malloc(sizeof(float) * thread_count);
    float* A = (float*)malloc(sizeof(float) * thread_count);
    memset(result, 0, sizeof(float) * thread_count);

    if (partitionPattern != NULL) active_threads = 0;
    if (partitionPattern != NULL) fprintf(stderr, "\n");

    for (int i = 0; i < thread_count; i++)
    {
        if (partitionPattern == NULL) {
            // divergence test
            if ((i / wave) % 2 == 0) A[i] = 0.2f;
            else A[i] = 0.8f;
        }
        else
        {
            if (partitionPattern[(i / wave)]) {
                A[i] = 0.2f;
                fprintf(stderr, "a ");
                active_threads++;
            }
            else
            {
                fprintf(stderr, "_ ");
                A[i] = 1.2f;
            }

            if ((i + 1) % wave == 0)
            {
                fprintf(stderr, "\n");
            }
        }
    }

    if (partitionPattern != NULL) fprintf(stderr, "\nActive threads: %d\n", active_threads);

    cl_mem a_mem_obj = clCreateBuffer(context, CL_MEM_READ_ONLY, thread_count * sizeof(float), NULL, &ret);
    cl_mem result_obj = clCreateBuffer(context, CL_MEM_READ_WRITE, thread_count * sizeof(float), NULL, &ret);
    ret = clEnqueueWriteBuffer(command_queue, a_mem_obj, CL_TRUE, 0, thread_count * sizeof(float), A, 0, NULL, NULL);
    ret = clEnqueueWriteBuffer(command_queue, result_obj, CL_TRUE, 0, thread_count * sizeof(float), result, 0, NULL, NULL);
    clSetKernelArg(kernel, 0, sizeof(cl_mem), (void*)&a_mem_obj);
    clSetKernelArg(kernel, 1, sizeof(cl_int), (void*)&chase_iterations);
    clSetKernelArg(kernel, 2, sizeof(cl_mem), (void*)&result_obj);
    clFinish(command_queue);

    // start with a low iteration count and try to make it work for all GPUs without needing manual iteration adjustment
    while (time_diff_ms < TARGET_TIME_MS / 2) {
        start_timing();
        ret = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &global_item_size, &local_item_size, 0, NULL, NULL);
        if (ret != CL_SUCCESS)
        {
            fprintf(stderr, "Failed to submit kernel to command queue. clEnqueueNDRangeKernel returned %d\n", ret);
            gOpsPerSec = 0;
            return 0;
        }

        ret = clFinish(command_queue);
        if (ret != CL_SUCCESS)
        {
            printf("Failed to finish command queue. clFinish returned %d\n", ret);
            gOpsPerSec = 0;
            return 0;
        }

        time_diff_ms = end_timing();

        totalOps = (float)chase_iterations * 8 * (float)active_threads;
        gOpsPerSec = ((float)totalOps / 1e9) / ((float)time_diff_ms / 1000);
        //fprintf(stderr, "chase iterations: %d, thread count: %d\n", chase_iterations, thread_count);
        //fprintf(stderr, "total ops: %f (%.2f G)\ntotal time: %llu ms\n", totalOps, totalOps / 1e9, time_diff_ms);

        chase_iterations = adjust_iterations(chase_iterations, time_diff_ms);
        clSetKernelArg(kernel, 1, sizeof(cl_int), (void*)&chase_iterations);
    }

    clReleaseMemObject(a_mem_obj);
    clReleaseMemObject(result_obj);
    free(A);
    free(result);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    return gOpsPerSec;
}

// often takes time for clocks to settle?
#define LATENCY_REPEAT 5
float run_latency_test(cl_context context,
    cl_command_queue command_queue,
    cl_kernel kernel,
    uint32_t chase_iterations,
    int float4_element_count,
    cl_mem a_mem_obj,
    cl_mem result_obj,
    cl_float* A,
    cl_float* result,
    float opsPerIteration)
{
    size_t global_item_size = 1;
    size_t local_item_size = 1;
    cl_int ret;
    float latency;
    uint64_t time_diff_ms = 0;

    // hack around latency taking longer
    chase_iterations = chase_iterations / 50;

    // testing returning a float4
    memset(result, 0, sizeof(float) * 4);

    ret = clEnqueueWriteBuffer(command_queue, a_mem_obj, CL_TRUE, 0, float4_element_count * sizeof(float), A, 0, NULL, NULL);
    ret = clEnqueueWriteBuffer(command_queue, result_obj, CL_TRUE, 0, sizeof(float) * 4, result, 0, NULL, NULL);
    clSetKernelArg(kernel, 0, sizeof(cl_mem), (void*)&a_mem_obj);
    clSetKernelArg(kernel, 1, sizeof(cl_int), (void*)&chase_iterations);
    clSetKernelArg(kernel, 2, sizeof(cl_mem), (void*)&result_obj);
    clFinish(command_queue);

    //fprintf(stderr, "Submitting fp32 add kernel to command queue\n");
    // start with a low iteration count and try to make it work for all GPUs without needing manual iteration adjustment
    while (time_diff_ms < TARGET_TIME_MS / 2) {
        start_timing();
        ret = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &global_item_size, &local_item_size, 0, NULL, NULL);
        if (ret != CL_SUCCESS)
        {
            fprintf(stderr, "Failed to submit kernel to command queue. clEnqueueNDRangeKernel returned %d\n", ret);
            latency = 0;
            return 0;
        }

        ret = clFinish(command_queue);
        if (ret != CL_SUCCESS)
        {
            printf("Failed to finish command queue. clFinish returned %d\n", ret);
            latency = 0;
            return 0;
        }

        time_diff_ms = end_timing();
        chase_iterations = adjust_iterations(chase_iterations, time_diff_ms);
        clSetKernelArg(kernel, 1, sizeof(cl_int), (void*)&chase_iterations);
    }

    float totalOps = (float)chase_iterations * opsPerIteration * (float)global_item_size;
    latency = (float)time_diff_ms * 1e6 / totalOps;
    // fprintf(stderr, "\tinitial run: %f ns latency\n", latency);

    float minLatency = 0.0f;
    for (int i = 0; i < LATENCY_REPEAT; i++)
    {
        start_timing();
        clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &global_item_size, &local_item_size, 0, NULL, NULL);
        clFinish(command_queue);
        time_diff_ms = end_timing();
        latency = (float)time_diff_ms * 1e6 / totalOps;
        // fprintf(stderr, "\trun %d: %f ns latency\n", i, latency);
        if (i == 0 || latency < minLatency) minLatency = latency;
    }

    //fprintf(stderr, "chase iterations: %d, thread count: %d\n", chase_iterations, thread_count);
    //fprintf(stderr, "total ops: %f (%.2f G)\ntotal time: %llu ms\n", totalOps, totalOps / 1e9, time_diff_ms);
    return minLatency;
}

// taking out FP64 because some implementations don't support it. putting another build program + create kernel section
// in the main instruction rate test function would be too messy
float fp64_instruction_rate_test(cl_context context,
    cl_command_queue command_queue,
    uint32_t thread_count,
    uint32_t local_size,
    uint32_t chase_iterations,
    int float4_element_count,
    cl_mem a_mem_obj,
    cl_mem result_obj,
    cl_float *A,
    cl_float*result)
{
    size_t global_item_size = thread_count;
    size_t local_item_size = local_size;
    float gOpsPerSec, totalOps;
    cl_int ret;
    int64_t time_diff_ms;

    // FP64 add test
    uint32_t low_chase_iterations = chase_iterations / 4;
    cl_double* fp64_A = (cl_double*)A;
    for (int i = 0; i < float4_element_count * 2; i++)
    {
        fp64_A[i] = 0.5f * i;
    }

    memset(result, 0, sizeof(float) * 4 * thread_count);

    cl_program program = build_program(context, "instruction_rate_fp64_kernel.cl", NULL);
    if (saveprogram) write_program(program, "fp64irate");
    cl_kernel fp64_add_rate_kernel = clCreateKernel(program, "fp64_add_rate_test", &ret);
    cl_kernel fp64_fma_rate_kernel = clCreateKernel(program, "fp64_fma_rate_test", &ret);
    cl_kernel fp64_mad_rate_kernel = clCreateKernel(program, "fp64_mad_rate_test", &ret);
    totalOps = 2.0f * 8.0f;
    gOpsPerSec = run_rate_test(context, command_queue, fp64_add_rate_kernel, thread_count, local_size, low_chase_iterations,
        float4_element_count, a_mem_obj, result_obj, A, result, totalOps);
    fprintf(stderr, "FP64 G Adds/sec: %f\n", gOpsPerSec);
    gOpsPerSec = run_rate_test(context, command_queue, fp64_fma_rate_kernel, thread_count, local_size, low_chase_iterations,
        float4_element_count, a_mem_obj, result_obj, A, result, totalOps);
    fprintf(stderr, "FP64 G FMAs/sec: %f : %f FP64 GFLOPs\n", gOpsPerSec, gOpsPerSec * 2);
    gOpsPerSec = run_rate_test(context, command_queue, fp64_mad_rate_kernel, thread_count, local_size, low_chase_iterations,
        float4_element_count, a_mem_obj, result_obj, A, result, totalOps);
    fprintf(stderr, "FP64 G mad()/sec: %f : %f FP64 GFLOPs\n", gOpsPerSec, gOpsPerSec * 2);

    return gOpsPerSec;
}

// taking out FP16 too because it requires an extension to be supported
float fp16_instruction_rate_test(cl_context context,
    cl_command_queue command_queue,
    uint32_t thread_count,
    uint32_t local_size,
    uint32_t chase_iterations,
    int float4_element_count,
    cl_mem a_mem_obj,
    cl_mem result_obj,
    cl_float* A,
    cl_float* result)
{
    size_t global_item_size = thread_count;
    size_t local_item_size = local_size;
    float gOpsPerSec, totalOps;
    cl_int ret;
    int64_t time_diff_ms;

    // FP64 add test
    uint32_t low_chase_iterations = chase_iterations / 4;
    cl_half* fp16_A = (cl_float*)A;
    for (int i = 0; i < float4_element_count * 8; i++)
    {
        fp16_A[i] = (cl_half)(0.5f * i);
    }

    memset(result, 0, sizeof(float) * 4 * thread_count);

    cl_program program = build_program(context, "instruction_rate_fp16_kernel.cl", NULL);
    if (saveprogram) write_program(program, "fp16irate");
    cl_kernel fp16_add_rate_kernel = clCreateKernel(program, "fp16_add_rate_test", &ret);
    cl_kernel fp16_fma_rate_kernel = clCreateKernel(program, "fp16_fma_rate_test", &ret);
    //cl_kernel fp16_rsqrt_rate_kernel = clCreateKernel(program, "fp16_rsqrt_rate_test", &ret);
    totalOps = 8.0f * 8.0f;
    gOpsPerSec = run_rate_test(context, command_queue, fp16_add_rate_kernel, thread_count, local_size, low_chase_iterations,
        float4_element_count, a_mem_obj, result_obj, A, result, totalOps);
    fprintf(stderr, "FP16 G Adds/sec: %f\n", gOpsPerSec);
    gOpsPerSec = run_rate_test(context, command_queue, fp16_fma_rate_kernel, thread_count, local_size, low_chase_iterations,
        float4_element_count, a_mem_obj, result_obj, A, result, totalOps);
    fprintf(stderr, "FP16 G FMAs/sec: %f : %f FP16 GFLOPs\n", gOpsPerSec, gOpsPerSec * 2);
    /*gOpsPerSec = run_rate_test(context, command_queue, fp16_rsqrt_rate_kernel, thread_count, local_size, low_chase_iterations,
        float4_element_count, a_mem_obj, result_obj, A, result, totalOps);
    fprintf(stderr, "FP16 G native_rsqrt/sec: %f\n", gOpsPerSec);*/

    return gOpsPerSec;
}
