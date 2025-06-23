# Guide for SSGX CMake Build System

##  Overview

The **SSGX Build Framework** provides a modular and reusable set of CMake APIs for building Intel SGX applications. It is designed to simplify:

- SGX SDK and SGXSSL configuration
- EDL processing (edger8r)
- Trusted and untrusted module generation
- Enclave signing workflows

All external integrations are exposed via a single public entrypoint: [`ssgx-build.cmake`](ssgx-build.cmake).

---

## Integration

In your `CMakeLists.txt`, import the framework automatically with SSGX framework:

```cmake
find_package(ssgx REQUIRED)
```

> Tip: This is designed to be compatible with `find_package(ssgx REQUIRED)` if installed system-wide.

---

## Configuration Setup

Before defining any enclave targets, set the build mode and hardware mode:

```cmake
ssgx_set_build_mode(Release)       # Options: Debug, PreRelease, Release
ssgx_set_hardware_mode(ON)       # Options: ON, OFF
```

These configure compiler flags and SGX runtime libraries automatically.

---

## Building SGX Components

### 1. Define a Trusted Static Library

```cmake
ssgx_add_trusted_library(my_enclave_lib
  SRCS enclave.cpp
  EDL my_enclave.edl
  EDL_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR}
  TRUSTED_LIBS common_tlib
)
```

> This compiles `.cpp` and generates EDL header via `sgx_edger8r`.

---

### 2. Define a Full Enclave (.so)

```cmake
ssgx_add_enclave_library(my_enclave
  SRCS enclave_main.cpp
  EDL my_enclave.edl
  TRUSTED_LIBS my_enclave_lib
  USE_SGXSSL
  LDSCRIPT enclave.lds
)
```

> This generates a signed-loadable enclave shared object. `USE_SGXSSL` links with `libsgx_tsgxssl.a`.

---

### 3. Define an Untrusted App or Library

#### App:

```cmake
ssgx_add_untrusted_executable(my_host_app
  SRCS host.cpp
  EDL my_enclave.edl
  UNTRUSTED_LIBS Poco::Foundation
)
```

#### Library:

```cmake
ssgx_add_untrusted_library(my_host_lib STATIC
  SRCS helper.cpp
  EDL my_enclave.edl
  UNTRUSTED_LIBS common_utils
)
```

> These APIs invoke `sgx_edger8r --untrusted`, generate `_u.c/.h`, and link all needed SGX runtime libraries.

---

### 4. Sign the Enclave

```cmake
ssgx_sign_enclave(my_enclave
  KEY enclave_private.pem
  CONFIG enclave.config.xml
  OUTPUT my_enclave.signed.so
)
```

> Generates `.signed.so` using `sgx_sign` one-step or two-step flow.

---

## Exposed Global Variables

You can read the following:

| Variable Name                   | Description                                |
|--------------------------------|--------------------------------------------|
| `SSGX_ENV__SGXSDK`             | Intel SGX SDK install path                 |
| `SSGX_ENV__SGXSSL`             | Intel SGX SSL install path                 |
| `SSGX_ENV__BUILD_MODE`         | One of: `Debug`, `PreRelease`, `Release`   |
| `SSGX_ENV__HARDWARE_MODE`      | Either `"ON"` or `"OFF"`                   |
| `SSGX_ENV__SGX_COMMON_CFLAGS`  | Global C/C++ compile flags for SGX modules |
| `SSGX_ENV__EDL_SEARCH_PATHS`   | Additional paths for EDL lookup            |
| `SSGX_ENV__CMAKE_ENTRY_PATH`   | Entry path of cmake                        |

> Tips: These variables are globally accessible and may be inspected for debugging or advanced customization. However, direct modification is discouraged. Instead, prefer using the provided `ssgx_set_*` functions to ensure compatibility and avoid unexpected behavior.
---

## Advanced Options

All `add_*` functions support advanced customization:

| Option            | Description                                  |
|-------------------|----------------------------------------------|
| `USE_SGXSSL`      | Link with Intel SGX SSL inside enclave       |
| `LDSCRIPT`        | Custom linker script for enclave             |
| `EDL_SEARCH_PATHS`| Lookup paths for locating `.edl` files       |
| `TRUSTED_LIBS`    | Trusted library dependencies (recursive)     |
| `UNTRUSTED_LIBS`  | Untrusted library dependencies (recursive)   |
| `USE_PREFIX`      | Prefix EDL-generated headers with target name|

