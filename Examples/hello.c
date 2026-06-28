#ifndef CL_TARGET_OPENCL_VERSION
#define CL_TARGET_OPENCL_VERSION 100
#endif

#include <stdio.h>
#include <string.h>
#include <CL/cl.h>

static void print_device_info(cl_device_id d) {
	char name[128], vendor[128];
	cl_device_type typ;
	name[0] = 0; vendor[0] = 0;
	clGetDeviceInfo(d, CL_DEVICE_NAME, sizeof(name), name, NULL);
	clGetDeviceInfo(d, CL_DEVICE_VENDOR, sizeof(vendor), vendor, NULL);
	clGetDeviceInfo(d, CL_DEVICE_TYPE, sizeof(typ), &typ, NULL);
	printf("  %s - %s (", name, vendor);
	if (typ & CL_DEVICE_TYPE_GPU) printf("GPU");
	else if (typ & CL_DEVICE_TYPE_ACCELERATOR) printf("Accelerator");
	else if (typ & CL_DEVICE_TYPE_CPU) printf("CPU");
	else printf("Unknown");
	printf(")");
}

int main() {
	cl_uint platform_count, pi, di, device_count;
	cl_platform_id platforms[8];
	cl_context context;
	cl_command_queue queue;
	cl_program program;
	cl_kernel kernel;
	cl_mem buffer_in, buffer_out;
	cl_int error;
	char* result;
	const char* sources[1];
	size_t global_size[1];
	const char* program_code;
	const char* transformed;
	size_t transformed_length;
	cl_device_id devices[8];

	error = clGetPlatformIDs(0, NULL, &platform_count);
	if (error != CL_SUCCESS || platform_count == 0) { fprintf(stderr, "No OpenCL platforms.\n"); return 1; }
	if (platform_count > 8) platform_count = 8;
	clGetPlatformIDs(platform_count, platforms, &platform_count);

	transformed = "Khoor#Zruog$";
	transformed_length = strlen(transformed);
	global_size[0] = transformed_length;

	program_code =
		"__kernel void decode(__global uchar* in, __global uchar* out)\n"
		"{\n"
		"	size_t i = get_global_id(0);\n"
		"	out[i] = in[i] - 3;\n"
		"}\n"
	;

	for (pi = 0; pi < platform_count; pi++) {
		{	cl_context_properties props[3];
			props[0] = CL_CONTEXT_PLATFORM;
			props[1] = (cl_context_properties)platforms[pi];
			props[2] = 0;
			context = clCreateContextFromType(props, CL_DEVICE_TYPE_ALL, NULL, NULL, &error);
			if (!context) continue;
		}
		{	size_t sz;
			clGetContextInfo(context, CL_CONTEXT_DEVICES, 0, NULL, &sz);
			device_count = (cl_uint)(sz / sizeof(cl_device_id));
			if (device_count > 8) device_count = 8;
			clGetContextInfo(context, CL_CONTEXT_DEVICES, device_count * sizeof(cl_device_id), devices, NULL);
		}

		sources[0] = program_code;
		program = clCreateProgramWithSource(context, 1, sources, NULL, &error);
		if (!program) { clReleaseContext(context); continue; }

		buffer_in = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, transformed_length, (void*)transformed, NULL);
		buffer_out = clCreateBuffer(context, CL_MEM_WRITE_ONLY, transformed_length, NULL, NULL);
		if (!buffer_in || !buffer_out) { clReleaseProgram(program); clReleaseContext(context); continue; }

		for (di = 0; di < device_count; di++) {
			error = clBuildProgram(program, 1, &devices[di], "", NULL, NULL);
			if (error) {
				char log[4096]; size_t ls = 0;
				log[0] = 0;
				clGetProgramBuildInfo(program, devices[di], CL_PROGRAM_BUILD_LOG, sizeof(log), log, &ls);
				continue;
			}
			kernel = clCreateKernel(program, "decode", &error);
			if (!kernel) continue;

			clSetKernelArg(kernel, 0, sizeof(buffer_in), &buffer_in);
			clSetKernelArg(kernel, 1, sizeof(buffer_out), &buffer_out);

			queue = clCreateCommandQueue(context, devices[di], 0, &error);
			if (!queue) { clReleaseKernel(kernel); continue; }

			clEnqueueNDRangeKernel(queue, kernel, 1, NULL, global_size, NULL, 0, NULL, NULL);
			result = (char*)clEnqueueMapBuffer(queue, buffer_out, CL_TRUE, CL_MAP_READ, 0, transformed_length, 0, NULL, NULL, NULL);

			print_device_info(devices[di]);
			printf("\n  in: %.*s\n  out: %.*s\n\n", (int)transformed_length, transformed, (int)transformed_length, result);

			clEnqueueUnmapMemObject(queue, buffer_out, result, 0, NULL, NULL);
			clReleaseCommandQueue(queue);
			clReleaseKernel(kernel);
		}

		clReleaseMemObject(buffer_in);
		clReleaseMemObject(buffer_out);
		clReleaseProgram(program);
		clReleaseContext(context);
	}

	return 0;
}
