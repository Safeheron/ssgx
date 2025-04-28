#ifndef _SSGX_UATTESTATION_UCOMMON_H_
#define _SSGX_UATTESTATION_UCOMMON_H_

#include <stdint.h>
#include <string>

namespace ssgx {
namespace attestation_u {

// Supplemental data version
typedef union _supp_ver_t {
    uint32_t version;
    struct {
        uint16_t major_version;
        uint16_t minor_version;
    };
} supp_ver_t;

int initialize_qe_setting(bool is_out_of_proc = false);
int clean_qe_setting(bool is_out_of_proc = false);

} // namespace attestation_u
} // namespace ssgx

#endif //_SSGX_UATTESTATION_UCOMMON_H_