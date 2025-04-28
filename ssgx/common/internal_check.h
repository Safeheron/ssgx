#ifndef SSGXLIB_INTERNAL_CHECK_H
#define SSGXLIB_INTERNAL_CHECK_H

#include <cstdint>
#include "sgx_key.h"
#include "sgx_attributes.h"

// refer to: https://github.com/intel/linux-sgx/blob/main/sdk/tseal/tSeal_internal.h
#define KEY_POLICY_KSS (SGX_KEYPOLICY_CONFIGID | SGX_KEYPOLICY_ISVFAMILYID | SGX_KEYPOLICY_ISVEXTPRODID)

// refer to: https://github.com/intel/linux-sgx/blob/7385e10ce1106215d15f874a024ca224c7417eea/sdk/tseal/tSeal.cpp#L96
static bool is_valid_key_policy(uint16_t key_policy) {
    // check key_request->key_policy:
    //  1. Reserved bits are not set
    //  2. Either MRENCLAVE or MRSIGNER is set
    // NOLINTNEXTLINE(readability-simplify-boolean-expr, readability-implicit-bool-conversion)
    return !((key_policy &
              ~(SGX_KEYPOLICY_MRENCLAVE | SGX_KEYPOLICY_MRSIGNER | (KEY_POLICY_KSS) | SGX_KEYPOLICY_NOISVPRODID)) ||
             (key_policy & (SGX_KEYPOLICY_MRENCLAVE | SGX_KEYPOLICY_MRSIGNER)) == 0);
}

// refer to: https://github.com/intel/linux-sgx/blob/7385e10ce1106215d15f874a024ca224c7417eea/sdk/tseal/tSeal.cpp#L101
static bool is_valid_attribute_mask(sgx_attributes_t attribute_mask) {
    // NOLINTNEXTLINE(readability-simplify-boolean-expr, readability-implicit-bool-conversion)
    return !(!(attribute_mask.flags & SGX_FLAGS_INITTED) || !(attribute_mask.flags & SGX_FLAGS_DEBUG));
}

#endif // SSGXLIB_INTERNAL_CHECK_H
