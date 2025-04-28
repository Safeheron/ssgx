# Features

## CMake-Integrated SGX Build Framework

To support the separation of [Enclave](./test/BasicTest/enc/CMakeLists.txt) and [App](./test/BasicTest/host/CMakeLists.txt) architectures, this framework builds a CMake-compatible compilation system,
optimizing modular development, dependency management, and cross-component integration, significantly reducing the
complexity of SGX project builds.

## Key System Functionality Supplements

To address the limitations of SGX’s standard library support, this framework provides:

- [High-precision time support (millisecond, microsecond, nanosecond levels)](./test/BasicTest/cases/ssgx_utils_t_time_test.cpp).
- [Secure untrusted memory allocation](./test/BasicTest/cases/ssgx_utils_t_test.cpp), optimizing Enclave-to-App data interactions while ensuring secure access.
- [File system access support](./test/BasicTest/cases/ssgx_filesystem_t_test.cpp), providing essential file system interaction capabilities.

## Advanced Utility Extensions

To mitigate the lack of third-party libraries in the SGX environment, the framework offers:

- [TOML configuration file management](./test/BasicTest/cases/ssgx_config_t_test.cpp), ensuring secure configuration parsing and management.
- [JSON parsing support](./test/BasicTest/cases/ssgx_json_t_test.cpp), delivering efficient JSON handling for improved data exchange.
- [High-precision numerical computation](./test/BasicTest/cases/ssgx_decimal_t_test.cpp), applicable to cryptographic and financial applications requiring precise
  calculations.
- [Logging system support](./test/BasicTest/cases/ssgx_log_t_test.cpp), offering an SGX-compatible logging framework for debugging and error analysis.
- HTTP(s) functionality, encapsulating secure [HTTPs client](./test/BasicTest/cases/ssgx_http_t_client_test.cpp) and [HTTP server](./test/HttpTest/enc/Enclave.cpp), enhancing Enclave's
  networking capabilities.
- [UUID Version 4 generation](./test/BasicTest/cases/ssgx_utils_t_uuid_test.cpp), providing random unique identifiers for various application needs.

## SGX TEE Testing Framework

To address the challenges of testing SGX code, the framework provides:

- [Unit and integration testing](./test/BasicTest/cases/ssgx_testframework_t_test.cpp) in a trusted environment, improving test coverage and stability.
- A secure testing environment, allowing functionality validation without compromising Enclave isolation.

## Secure and Encrypted File I/O

The framework enhances object-oriented file stream support, extending beyond the MRSIGNER mechanism to incorporate
multiple encryption schemes:

- [MRENCLAVE-bound secure storage](./test/BasicTest/cases/ssgx_filesystem_t_test.cpp), ensuring that data is accessible only by a specific Enclave.
- [Flexible key derivation and encryption mechanisms](./test/BasicTest/cases/ssgx_filesystem_t_test.cpp), improving file storage security and compatibility.

## Intuitive SGX API Design with OOP

To reduce the complexity of core SGX function interfaces, the framework provides object-oriented API encapsulations for
key SGX features:

- [Sealing (Secure Storage)](./test/BasicTest/cases/ssgx_utils_t_seal_handler_test.cpp): High-level APIs for simplified encrypted storage and key management.
- [Remote Attestation](./test/BasicTest/cases/ssgx_attestation_t_test.cpp): Encapsulated SGX remote attestation processes, making trust verification more intuitive and easy
  to use.