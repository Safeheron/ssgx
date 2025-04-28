# Example: Seal

## Overview

This project demonstrates how to use the Safeheron SGX CMake framework (`ssgx`) to build a sample SGX application that performs **data sealing and unsealing**. It shows how SGX sealing capabilities can be used to:

- Seal sensitive data in memory with enclave-specific keys (`MRENCLAVE`, `MRSIGNER`)
- Persist sealed data to disk and later decrypt it
- Use Intel SGX's local sealing APIs to protect file contents and custom data buffers

This example uses Safeheron's modular `ssgx` build system and runs in Intel SGX hardware or simulation mode.

---

## Build Instructions

### 1. Configure & Build

```bash
cmake --preset release-config
cmake --build --preset release-build
```

This will create a build directory (e.g. `release-config`) and generate all necessary SGX components:
- Host application
- Enclave shared object
- Sealing logic
- EDL interface code

### 2. Run the Sample

```bash
cd release-config
./host/seal_app ./enc/seal_enclave.signed.so
```

Expected output:
```
Try to create testing enclave ...
Enclave is created!

Sample 1 start
Sample 1 execution successfully.
Sample 2 start
Sample 2 execution successfully.
Sample 3 start
Sample 3 execution successfully.
Sample 4 start
Sample 4 execution successfully.
Exit from function ecall_run()!
```

---

## Notes

- Requires Intel SGX SDK installed at `/opt/intel/sgxsdk`
- Requires Safeheron SGX Development Framework installed at `/opt/safeheron/ssgx`
- Requires SGX-compatible hardware or simulation driver
- The sealing key is bound to `MRENCLAVE`, `MRSIGNER` or more flexible, ensuring confidentiality and integrity even across restarts
