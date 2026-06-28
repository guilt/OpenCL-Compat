# Minimal OpenCL program on Windows

This is a project to demonstrate a minimal OpenCL project on Windows, using
the [Khronos OpenCL-Headers](https://github.com/KhronosGroup/OpenCL-Headers)
(v2026.05.29) with modifications for standalone use and no GL dependency by
default.

## Header Overview

The `Include/CL/` directory contains Khronos OpenCL headers. The dependency
graph is a strict DAG rooted at `cl_platform.h`:

```
cl_platform.h  (types, macros, platform abstractions)
    |
    +---> cl.h  (core OpenCL C API declarations)
    |          |
    |          +---> cl_ext.h  (extension definitions, e.g. cl_khr_fp16)
    |          +---> cl_gl.h  (OpenGL interop)      -- guarded by CL_HPP_USE_GL_INTEROP
    |          +---> cl_gl_ext.h  (GL ext interop)   -- guarded by CL_HPP_USE_GL_INTEROP
    |          +---> cl_d3d10.h  (Direct3D 10 interop)
    |          +---> cl_d3d11.h  (Direct3D 11 interop)
    |          +---> cl_dx9_media_sharing.h  (DX9 media interop)
    |          |
    |          +---> opencl.h  (convenience: includes all of the above)
    |
    +---> cl.hpp  (C++ wrapper, includes cl.h + cl_ext.h)
    |          |
    |          +---> opencl.hpp  (C++ convenience, includes cl.hpp)
    |          +---> cl2.hpp     (alias for cl.hpp)
```

### Choosing what to include

| Language | Scenario | Include | Notes |
|---|---|---|---|
| C | Core only | `<CL/cl.h>` | No external dependencies |
| C | Core + extensions | `<CL/cl_ext.h>` | Adds extension macros |
| C | Core + GL interop | `-DCL_HPP_USE_GL_INTEROP` + `<CL/cl.h>` or `<CL/opencl.h>` | No OpenGL SDK needed, types defined in cl_gl.h |
| C | Everything | `<CL/opencl.h>` | With `-DCL_HPP_USE_GL_INTEROP` for GL support |
| C++ | Core only | `<CL/cl.hpp>` | No GL or D3D dependencies |
| C++ | With GL interop | `-DCL_HPP_USE_GL_INTEROP` + `<CL/cl.hpp>` | No OpenGL SDK needed |
| C++ | With D3D10/DX9 interop | `-DCL_HPP_USE_DX_INTEROP` + `<CL/cl.hpp>` | Windows only |
| C++ | With D3D11 interop | `#include <CL/cl_d3d11.h>` | Windows only, standalone header |

### Feature flags

Define these **before** including any OpenCL header:

| Flag | Effect |
|---|---|
| `CL_TARGET_OPENCL_VERSION` | Targets a specific OpenCL version (100–310). Defaults to 310 (OpenCL 3.1). Set to 110 for OpenCL 1.1 compatibility. Used by `cl_version.h` to expose only the relevant APIs. |
| `CL_HPP_USE_GL_INTEROP` | Enables OpenGL interop: includes `cl_gl.h`, and in C++ mode exposes `BufferGL`, `ImageGL`, `enqueueAcquireGLObjects`, etc. No OpenGL SDK required. |
| `CL_HPP_USE_DX_INTEROP` | (C++ only) Enables Direct3D 10 / DX9 media sharing interop classes in `cl.hpp`. Windows only. |
| `USE_GL_INTEROP` | Deprecated alias for `CL_HPP_USE_GL_INTEROP`. |
| `USE_DX_INTEROP` | Deprecated alias for `CL_HPP_USE_DX_INTEROP`. |

### Notes

- `cl_gl.h` does **not** require the OpenGL SDK — it forward-declares `struct __GLsync` and defines `cl_GLint`/`cl_GLenum`/`cl_GLuint` locally.
- `cl_gl_ext.h` is a heritage header from older Khronos releases; its contents have been merged into `cl_gl.h`.
- `cl_d3d10.h` and `cl_d3d11.h` are guarded by `_WIN32` — both the system header include and all D3D type-dependent typedefs are only compiled on Windows.
- `cl_dx9_media_sharing.h` includes `<d3d9.h>` only under `_WIN32` (upstream Khronos convention).
- Apple platforms always include GL interop headers (`CL_HPP_USE_GL_INTEROP` is implicit on `__APPLE__`).
- `CL_HPP_USE_GL_INTEROP` is our custom flag (not from Khronos). `CL_HPP_USE_DX_INTEROP` follows the upstream
  Khronos C++ wrapper naming. The deprecated `USE_DX_INTEROP` is mapped automatically.

## MinGW Instructions

Use `Lib/x86` for 32-bit MinGW or `Lib/x86_64` for 64-bit MinGW.

```shell
	gcc -I Include -L Lib/x86 Examples\hello.c -o Bin/hello.exe -lopenCL
```

For C++, on Windows XP, Ensure you have [MinGW Compat Headers](https://github.com/guilt/MinGW-Compat)
cloned, generated and `CPLUS_INCLUDE_PATH` set to.


```shell
	g++ -I Include -L Lib/x86 Examples\print_info.cpp -o Bin/print_info.exe -lopenCL
```

For GL interop, add `-DCL_HPP_USE_GL_INTEROP`:

```shell
	g++ -DCL_HPP_USE_GL_INTEROP -I Include -L Lib/x86 Examples\print_info.cpp -o Bin/print_info_gl.exe -lopenCL
```

For D3D10/DX9 interop, add `-DCL_HPP_USE_DX_INTEROP`:

```shell
	g++ -DCL_HPP_USE_DX_INTEROP -I Include -L Lib/x86 Examples\print_info.cpp -o Bin/print_info_dx.exe -lopenCL
```

All interop flags can be combined:

```shell
	g++ -DCL_HPP_USE_GL_INTEROP -DCL_HPP_USE_DX_INTEROP -I Include -L Lib/x86 Examples\print_info.cpp -o Bin/print_info_all.exe -lopenCL
```

## Windows XP Compatibility

AMD's OpenCL for Windows XP exports **81 functions** (OpenCL 1.0 + 1.1 + a few 1.2 additions like
`clCreateUserEvent`, `clSetUserEventStatus`, `clSetEventCallback`).

**Functions NOT available in the XP DLL** (29 total):

| OpenCL 1.2 | OpenCL 2.0 |
|---|---|
| `clCompileProgram` | `clCreateCommandQueueWithProperties` |
| `clCreateFromGLTexture` | `clCreatePipe` |
| `clCreateImage` | `clCreateSamplerWithProperties` |
| `clCreateProgramWithBuiltInKernels` | `clEnqueueSVMFree` |
| `clCreateSubDevices` | `clEnqueueSVMMap` |
| `clEnqueueBarrierWithWaitList` | `clEnqueueSVMMemcpy` |
| `clEnqueueFillBuffer` | `clEnqueueSVMMemFill` |
| `clEnqueueFillImage` | `clEnqueueSVMUnmap` |
| `clEnqueueMarkerWithWaitList` | `clGetPipeInfo` |
| `clEnqueueMigrateMemObjects` | `clSVMAlloc` |
| `clGetExtensionFunctionAddressForPlatform` | `clSVMFree` |
| `clGetKernelArgInfo` | `clSetKernelArgSVMPointer` |
| `clLinkProgram` | `clSetKernelExecInfo` |
| `clReleaseDevice` | |
| `clRetainDevice` | |
| `clUnloadPlatformCompiler` | |

Apps using any of these functions will **fail to start** on the XP driver with
"entry point not found". To avoid this, always define `CL_TARGET_OPENCL_VERSION`
to the version you target:

```c
#define CL_TARGET_OPENCL_VERSION 110  // or 100, 120, 200, etc.
#include <CL/cl.h>
```

For C++ via `cl.hpp`, also set the C++ wrapper versions:

```c
#define CL_TARGET_OPENCL_VERSION 110
#define CL_HPP_TARGET_OPENCL_VERSION 110
#define CL_HPP_MINIMUM_OPENCL_VERSION 110
#include <CL/cl.hpp>
```

### Notes

- On the AMD XP driver, `clGetDeviceInfo` and `clGetDeviceIDs` may crash before
  a context is created. Always create a context (and command queue) first, then
  query device properties. Both `hello.c` and `print_info.cpp` follow this order.
- `clEnqueueFillBuffer` (OpenCL 1.2) is **not** in the XP DLL. Projects like
  CLBlast that require it cannot run on this XP driver.
- The import library (`Lib/x86/libOpenCL.a`) exports all 110 functions up to
  OpenCL 2.0, but the linker only adds import entries for functions actually
  referenced by your code. Unused functions don't affect load-time behavior.

## More Details

More details in [this article](http://arkanis.de/weblog/2014-11-25-minimal-opencl-development-on-windows).
