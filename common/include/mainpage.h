/*!
 * @mainpage Safeheron-SGX-Native-Development-Framework
 *
 * <table>
 * <tr><th>Project  <td>Safeheron-SGX-Native-Development-Framework (SSGX)
 * <tr><th>Author   <td>Safeheron
 * <tr><th>Source   <td>https://github.com/Safeheron/ssgx
 * </table>
 *
 * @section Description Description
 *
   SSGX is a native development framework that leverages the hardware-enforced security guarantees of Intel SGX CPUs to enable trusted execution for sensitive applications.

 * @section Features Features
 *      - ***CMake-Integrated SGX Build Framework:***               To support the separation of Enclave and App architectures, this framework builds a CMake-compatible compilation system, optimizing modular development, dependency management, and cross-component integration, significantly reducing the complexity of SGX project builds.
 *      - ***Key System Functionality Supplements:***               We provides some system functions that Intel SGX does not provide, such as High-precision time, Secure Untrusted Memory Operations, File System Access.
 *      - ***Advanced Utility Extensions:***                        We add some useful functions that can be used in trusted environments, such as TOML configuration file management, JSON, High-precision numerical computation, Logging system, HTTP functionality.
 *      - ***SGX TEE Testing Framework:***                          We Provide unit and integration testing capabilities suitable for use in trusted environments.
 *      - ***Secure and Encrypted File I/O:***                      We extended the original protected_fs function of SGX to support secure storage in multiple binding modes such as mrsigner and mrenclave, ensuring that only specific enclaves can access data in protected files.
 *      - ***Intuitive SGX API Design with OOP:***                  We package the seal and remote attestation functions in the SGX API to make them easier to accept and use.
 *      - ***Advanced Cryptographic Support for Blockchain:***      This framework implements core cryptographic algorithms for blockchain, providing fundamental support for Multi-Party Computation (MPC) protocols and Zero-Knowledge Proof (ZKP) protocols.
 *
 *
 * @section Update Library Updates
 * <table>
 * <tr><th>Date         <th>Version      <th>Description  </tr>
 * <tr><td>2025/04/24   <td>1.2.0        <td>Initial release </tr>
 * </table>
 *
 *
 * @section Contribute Contribute & Contact
 * If you discover any bugs or have other issues and ideas, you can file an issue on [GitHub](https://github.com/Safeheron/ssgx/issues).
 * If you would like to learn more about Safeheron, you can visit [safeheron.com](https://safeheron.com/) or [GitHub](https://github.com/Safeheron).
 * We look forward to hearing from you!
 *
 */


#ifndef SSGXLIB_MAINPAGE_H
#define SSGXLIB_MAINPAGE_H

#endif // SSGXLIB_MAINPAGE_H
