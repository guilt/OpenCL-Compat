#ifndef CL_TARGET_OPENCL_VERSION
#define CL_TARGET_OPENCL_VERSION 100
#endif
#ifndef CL_HPP_TARGET_OPENCL_VERSION
#define CL_HPP_TARGET_OPENCL_VERSION 100
#endif
#ifndef CL_HPP_MINIMUM_OPENCL_VERSION
#define CL_HPP_MINIMUM_OPENCL_VERSION 100
#endif

#include <CL/cl.hpp>
#include <iostream>

static void print_device(const cl::Device& device) {
    auto name = device.getInfo<CL_DEVICE_NAME>();
    auto vendor = device.getInfo<CL_DEVICE_VENDOR>();
    auto version = device.getInfo<CL_DEVICE_VERSION>();
    auto typ = device.getInfo<CL_DEVICE_TYPE>();
    auto workItems = device.getInfo<CL_DEVICE_MAX_WORK_ITEM_SIZES>();
    auto workGroups = device.getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>();
    auto computeUnits = device.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>();
    auto globalMemory = device.getInfo<CL_DEVICE_GLOBAL_MEM_SIZE>();
    auto localMemory = device.getInfo<CL_DEVICE_LOCAL_MEM_SIZE>();

    std::cout
    << "\n  Name: " << name
    << "\n  Vendor: " << vendor
    << "\n  Version: " << version
    << "\n  Type: ";
    if (typ & CL_DEVICE_TYPE_GPU) std::cout << "GPU ";
    if (typ & CL_DEVICE_TYPE_CPU) std::cout << "CPU ";
    if (typ & CL_DEVICE_TYPE_ACCELERATOR) std::cout << "Accelerator ";
#ifdef CL_DEVICE_TYPE_CUSTOM
    if (typ & CL_DEVICE_TYPE_CUSTOM) std::cout << "Custom ";
#endif
    std::cout
    << "\n  Max work-item sizes: (" << workItems[0] << "," << workItems[1] << "," << workItems[2] << ")"
    << "\n  Max work-group size: " << workGroups
    << "\n  Compute units: " << computeUnits
    << "\n  Global memory: " << (globalMemory >> 20) << " MB"
    << "\n  Local memory per CU: " << (localMemory >> 10) << " KB"
    << std::endl;
}

int main(){
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);

    if (platforms.empty()){
        std::cerr << "No platforms found!" << std::endl;
        return -1;
    }

    int device_count = 0;
    for (auto& platform : platforms) {
        auto name = platform.getInfo<CL_PLATFORM_NAME>();
        auto version = platform.getInfo<CL_PLATFORM_VERSION>();
        std::cout << "Platform: " << name << " (" << version << ")" << std::endl;

        std::vector<cl::Device> devices;
        platform.getDevices(CL_DEVICE_TYPE_ALL, &devices);
        for (auto& device : devices) {
            print_device(device);
            device_count++;
        }
        std::cout << std::endl;
    }

    std::cout << "\nTotal: " << device_count << " device(s) found." << std::endl;
    return device_count > 0 ? 0 : -1;
}
