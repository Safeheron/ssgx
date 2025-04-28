#ifndef _SAFEHERON_SGX_LIBRARY_CUSTOM_ASSERT_H_
#define _SAFEHERON_SGX_LIBRARY_CUSTOM_ASSERT_H_

#include "ssgx_exception_t.h"

#ifdef ASSERT_THROW
#error "ASSERT_THROW redefined"
#endif

#ifdef ASSERT_RETURN_FALSE
#error "ASSERT_RETURN_FALSE redefined"
#endif

#ifdef ASSERT_RETURN_MINUS_ONE
#error "ASSERT_RETURN_MINUS_ONE redefined"
#endif

#ifdef ASSERT_RETURN_CODE
#error "ASSERT_RETURN_CODE redefined"
#endif

#define ASSERT_THROW(boolean_expression)                                                                               \
    do {                                                                                                               \
        if (!(boolean_expression))                                                                                     \
            throw ssgx::exception_t::LocatedException(__FILE__, __LINE__, __FUNCTION__, -1, #boolean_expression);      \
    } while (0)

#define ASSERT_RETURN_FALSE(boolean_expression)                                                                        \
    do {                                                                                                               \
        if (!(boolean_expression))                                                                                     \
            return false;                                                                                              \
    } while (0)

#define ASSERT_RETURN_MINUS_ONE(boolean_expression)                                                                    \
    do {                                                                                                               \
        if (!(boolean_expression))                                                                                     \
            return -1;                                                                                                 \
    } while (0)

#define ASSERT_RETURN_CODE(boolean_expression, err_code)                                                               \
    do {                                                                                                               \
        if (!(boolean_expression))                                                                                     \
            return err_code;                                                                                           \
    } while (0)

#endif //_SAFEHERON_SGX_LIBRARY_CUSTOM_ASSERT_H_
