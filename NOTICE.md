# Third-Party Libraries and Components

---

## Third-Party Libraries Used
This project includes portions of code adapted from the following open-source projects. Their original licenses and copyright notices are preserved.

- [httpparser](https://github.com/nekipelov/httpparser)  
  License: **MIT**  
  Portions of the `Response` class have been reused in this project.

- [musl-libc](https://git.musl-libc.org/cgit/musl/tree)  
  License: **MIT**  
  Certain time-related utility functions have been adopted for use inside the TEE environment.

- [uuid4](https://github.com/rxi/uuid4)  
  License: **MIT**  
  Basic UUID generation logic has been referenced and integrated.

- [SGX-CMake](https://github.com/xzhangxa/SGX-CMake)  
  License: **BSD-3-Clause**  
  Some CMake build scripts have been adopted and integrated into this project to support SGX-specific compilation.

---

## Libraries Ported to SGX TEE
The following libraries have been ported and adapted to run within the SGX Trusted Execution Environment (TEE):

- [mpdecimal](https://www.bytereef.org/software/mpdecimal/releases/mpdecimal-4.0.0.tar.gz)  
  License: **BSD**  
  A package for correctly-rounded arbitrary-precision decimal floating point arithmetic.

- [nlohmann/json](https://github.com/nlohmann/json)  
  License: **MIT**  
  A modern and expressive C++ JSON library.

- [safeheron-crypto-suites-cpp](https://github.com/Safeheron/safeheron-crypto-suites-cpp)  
  License: **Safeheron Custom License**  
  Cryptographic primitives developed and maintained by Safeheron.  