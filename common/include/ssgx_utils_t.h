#ifndef SAFEHERON_SGX_TRUSTED_UTILS_H
#define SAFEHERON_SGX_TRUSTED_UTILS_H

#include <cstdint>
#include <cstdio>
#include <string>

#include "sgx_eid.h"

#include "ssgx_utils_t_seal_handler.h"
#include "ssgx_utils_t_time.h"
#include "ssgx_utils_t_uuid.h"
#include "ssgx_utils_t_enclave_info.h"

namespace ssgx {

/**
 * @namespace ssgx::utils_t
 * @brief This module provides a comprehensive set of utility classes and functions for secure memory management, time
 * handling, formatted operations, thread management, and other foundational functionalities in a Trusted Execution
 * Environment (TEE).
 *
 * The main functions are as follows:
 * - Secure allocation, deallocation, and operations for memory outside of TEE.
 * - Time-related operations classes.
 * - Formatted printing and string formatting operations.
 * - Thread operations, such as the sleep function.
 * - Unique identifier generation.
 *
 * @note Although a method is available to obtain the time within the trusted environment, the time source depends on
 * the operating system, which could be manipulated by an attacker. Consequently, the correctness of the retrieved time
 * cannot be guaranteed. Similarly, the sleep method cannot reliably ensure the specified sleep duration. In the future,
 * we plan to develop a secure mechanism for time retrieval.
 */
namespace utils_t {

/**
 * @brief Print formatted data to stdout.
 *
 * Writes the C string pointed by format to the standard output (stdout). If format includes format specifiers
 * (subsequences beginning with %), the additional arguments following format are formatted and inserted in the
 * resulting string replacing their respective specifiers.
 *
 * @param[in] format C string that contains the text to be written to stdout.
 * @return
 * - On success, the total number of characters written is returned.
 * - On failure, a negative number is returned.
 *
 * @code
 * @par Examples
 * @code
 *     printf ("Characters: %c %c \n", 'a', 65);
 *     printf ("Decimals: %d %ld\n", 1977, 650000L);
 *     printf ("Preceding with blanks: %10d \n", 1977);
 *     printf ("Preceding with zeros: %010d \n", 1977);
 *     printf ("Some different radices: %d %x %o %#x %#o \n", 100, 100, 100, 100, 100);
 *     printf ("floats: %4.2f %+.0e %E \n", 3.1416, 3.1416, 3.1416);
 *     printf ("Width trick: %*d \n", 5, 10);
 *     printf ("%s \n", "A string");
 *     int ret = printf ("A string");
 *     int len = strlen("A string");
 *     assert( ret == len );
 * @endcode
 * @see https://cplusplus.com/reference/cstdio/printf
 */
int Printf(const char* fmt, ...);

/**
 * @brief Format args according to the format string fmt, and return the result
 * as a string.
 *
 * @param[in] format C string that contains the text to be written to stdout.
 * @exception std::bad_alloc Throw the exception if memory allocation fails.
 * @exception std::invalid_argument Throw the exception if format string is null.
 * @exception std::runtime_error Throw the exception if formatting error in format_str.
 * @return formatted string.
 *
 * @par Examples
 * @code
 *     std::string result;
 *     result = FormatStr("Characters: %c %c \n", 'a', 65);
 *     result = FormatStr("Decimals: %d %ld\n", 1977, 650000L);
 *     result = FormatStr("Preceding with blanks: %10d \n", 1977);
 *     result = FormatStr("Preceding with zeros: %010d \n", 1977);
 *     result = FormatStr("Some different radices: %d %x %o %#x %#o \n", 100, 100, 100, 100, 100);
 *     result = FormatStr("floats: %4.2f %+.0e %E \n", 3.1416, 3.1416, 3.1416);
 *     result = FormatStr("Width trick: %*d \n", 5, 10);
 *     result = FormatStr("%s \n", "A string");
 * @endcode
 */
std::string FormatStr(const char* fmt, ...);

/**
 * @brief Allocates size bytes of uninitialized storage outside the enclave.
 *
 * - If allocation succeeds, returns a pointer that is suitably aligned for any object type with fundamental alignment.
 * - If size is zero, a null pointer will be returned. Alternatively, a non-null pointer may be returned; but such a
 * pointer should not be dereferenced, and should be passed to FreeOutside() to avoid memory leaks.
 *
 * @param[in] size    number of bytes to allocate
 * @return
 * - On success, returns the pointer to the beginning of newly allocated memory outside the enclave. To avoid a memory
 * leak, the returned pointer must be deallocated with FreeOutside().
 * - On failure, returns a null pointer.
 * @exception std::runtime_error Throw the exception if the allocated memory is not outside the Enclave.
 *
 * @par Examples
 * @code
 *      int *p1 = MallocOutside(4*sizeof(int));  // allocates enough for an array of 4 int
 *      int *p2 = MallocOutside(sizeof(int[4])); // same, naming the type directly
 *      int *p3 = MallocOutside(4*sizeof *p3);   // same, without repeating the type name
 *
 *      if(p1) {
 *          for(int n=0; n<4; ++n) // populate the array
 *              p1[n] = n*n;
 *          for(int n=0; n<4; ++n) // print it back out
 *              Printf("p1[%d] == %d\n", n, p1[n]);
 *      }
 *
 *      FreeOutside(p1);
 *      FreeOutside(p2);
 *      FreeOutside(p3);
 * @endcode
 */
void* MallocOutside(size_t size);

/**
 * @brief Allocates memory outside the enclave for an array of num objects of size and initializes all bytes in the
 * allocated storage to zero.
 *
 * - If allocation succeeds, returns a pointer to the lowest (first) byte in the allocated memory block that is suitably
 * aligned for any object type with fundamental alignment.
 * - If size is zero,  a null pointer will be returned. Alternatively, a non-null pointer may be returned; but such a
 * pointer should not be dereferenced, and should be passed to FreeOutside() to avoid memory leaks.
 *
 * @param[in] num     number of objects
 * @param[in] size    size of each object
 * @return
 * - On success, returns the pointer to the beginning of newly allocated memory outside the enclave. To avoid a memory
 * leak, the returned pointer must be deallocated with FreeOutside().
 * - On failure, returns a null pointer.
 * @exception std::runtime_error Throw the exception if the allocated memory is not outside the Enclave.
 *
 * @par Examples
 * @code
 *     int* p1 = CallocOutside(4, sizeof(int));    // allocate and zero out an array of 4 int
 *     int* p2 = CallocOutside(1, sizeof(int[4])); // same, naming the array type directly
 *     int* p3 = CallocOutside(4, sizeof *p3);     // same, without repeating the type name
 *
 *     if (p2)
 *     {
 *         for (int n = 0; n < 4; ++n) // print the array
 *             Printf("p2[%d] == %d\n", n, p2[n]);
 *     }
 *
 *     FreeOutside(p1);
 *     FreeOutside(p2);
 *     FreeOutside(p3);
 * @endcode
 */
void* CallocOutside(size_t num, size_t size);

/**
 * @brief Deallocates the space previously allocated by MallocOutside(),
 CallocOutside().
 *
 * - If ptr is a null pointer, the function does nothing.
 * - The behavior is undefined if the value of ptr does not equal a value returned earlier by MallocOutside(),
 CallocOutside().
 * - The behavior is undefined if the memory area referred to by ptr has already been deallocated, that is,
 FreeOutside() has already been called with ptr as the argument and no calls to MallocOutside(), CallocOutside()
 resulted in a pointer equal to ptr afterwards.
 * - The behavior is undefined if after FreeOutside() returns, an access is made through the pointer ptr (unless
 another allocation function happened to result in a pointer value equal to ptr).
 *
 * @param[in] ptr    pointer to the memory to deallocate.
 * @return (none)
 * @exception std::runtime_error Throw the exception if the allocated memory is not outside the Enclave.
 *
 * @par Examples
 * @code
    int *p1 = MallocOutside(10*sizeof *p1);
    FreeOutside(p1); // every allocated pointer must be freed

    int *p2 = CallocOutside(10, sizeof *p2);
    FreeOutside(p2);
 * @endcode
 */
void FreeOutside(void* ptr_outside, size_t size);

/**
 * @brief Duplicates a fixed-length C-style string to memory allocated outside the enclave.
 *
 * This function allocates a new buffer outside the enclave and copies exactly `str_len` bytes from
 * the input string `str_ptr`. A null terminator (`'\0'`) is appended at the end to ensure
 * the returned pointer is a valid null-terminated C string.
 *
 * Security check is performed using `strnlen(str_ptr, str_len + 1)` to ensure the input buffer
 * contains a valid string of exactly `str_len` characters.
 *
 * @param[in] str_ptr   Pointer to the source C-string (need to be null-terminated).
 * @param[in] str_len   Length of the string content (not including null terminator).
 * @return
 * - On success, returns a pointer to a null-terminated copy of the string allocated outside the enclave.
 * - On failure, returns a null pointer (e.g. invalid input, length mismatch, or allocation failure).
 *
 * @note The returned pointer must be freed using FreeOutside() to avoid memory leaks.
 *
 * @par Example
 * @code
 * const char* msg = "hello";
 * char* outside_str = StrndupOutside(msg, 5);
 * // Use outside_str...
 * FreeOutside(outside_str);
 * @endcode
 */
char* StrndupOutside(const char * str_ptr, std::size_t str_len);

/**
 * @brief Copies count bytes from the object pointed to by src to the object outside the enclave pointed to by
 dest_outside_enclave.
 *
 * Both objects are reinterpreted as arrays of unsigned char.
 * - If the objects overlap, the behavior is undefined.
 * - If either dest_outside_enclave or src is an invalid or null pointer, the behavior is undefined, even if count is
 zero.
 *
 * @param[in] dest_outside_enclave  pointer to the memory location to copy to.
 * @param[in] src   pointer to the memory location to copy from.
 * @param[in] count   number of bytes to copy.
 * @return  dest_outside_enclave
 * @exception std::runtime_error Throw the exception if the allocated memory is not outside the Enclave.
 *
 * @par Examples
 * @code
    char source[] = "once upon a daydream...",
    char *dest = CallocOutside(1, strlen(source) + 1);
    memcpy_to_outside_enclave(dest, source, strlen(source));
    FreeOutside(dest); // every allocated pointer must be freed
 * @endcode
 */
void* MemcpyToOutside(void* dest_outside_enclave, const void* src, std::size_t count);

/**
 * @brief Duplicates a raw memory buffer to memory allocated outside the enclave.
 *
 * This function allocates a buffer of `buf_size` bytes outside the enclave and performs
 * a direct byte-wise copy from the source `buf_ptr`.
 *
 * It is intended for transferring raw data (not necessarily strings) from enclave memory
 * to untrusted memory regions, such as for returning binary data in OCALLs.
 *
 * @param[in] buf_ptr   Pointer to the source memory.
 * @param[in] buf_size  Number of bytes to copy.
 * @return
 * - On success, returns a pointer to a newly allocated buffer outside the enclave containing a copy of the data.
 * - On failure, returns a null pointer.
 *
 * @note The returned pointer must be freed using FreeOutside() to avoid memory leaks.
 *
 * @par Example
 * @code
 * uint8_t data[4] = {1, 2, 3, 4};
 * void* outside_buf = MemdupOutside(data, sizeof(data));
 * // Use outside_buf...
 * FreeOutside(outside_buf);
 * @endcode
 */
void* MemdupOutside(const void* buf_ptr, std::size_t buf_size);

/**
 * @brief Sleep for a specified number of seconds using OCall.
 * @param seconds The number of seconds to sleep.
 */
void Sleep(uint32_t seconds);

} // namespace utils_t
} // namespace ssgx

#endif // SAFEHERON_SGX_TRUSTED_UTILS_H
