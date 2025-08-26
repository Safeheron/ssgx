#include <cstring>
#include <stdexcept>

#include "ssgx_attestation_t.h"
#include "ssgx_filesystem_t.h"
#include "ssgx_testframework_t.h"
#include "ssgx_utils_t.h"

#include "crypto-suites/crypto-encode/base64.h"
#include "crypto-suites/crypto-hash/sha256.h"
#include "Enclave_t.h"

using namespace ssgx::filesystem_t;
using namespace ssgx::attestation_t;
using namespace ssgx::utils_t;
using namespace safeheron::hash;

#define PRINT_REMOTE_ATTESTOR_STATUS(info, attestor)                                                                         \
    do {                                                                                                               \
        auto qv = (attestor).GetRawQvResult();                                                                         \
        auto err_code = (attestor).GetLastErrorCode();                                                                 \
        auto err_msg = (attestor).GetLastErrorMsg();                                                                   \
        if (qv.has_value()) {                                                                                          \
            ssgx::utils_t::Printf("[RemoteAttestor] %s:%d %s()\n"                                                      \
                                  "  Info         : %s\n"                                                          \
                                  "  QvResult     : 0x%04X\n"                                                          \
                                  "  ErrorCode    : %d\n"                                                              \
                                  "  ErrorMessage : %s\n",                                                             \
                                  __FILE__, __LINE__, __func__, info, static_cast<uint32_t>(*qv),                            \
                                  static_cast<int>(err_code), err_msg.c_str());                                        \
        } else {                                                                                                       \
            ssgx::utils_t::Printf("[RemoteAttestor] %s:%d %s()\n"                                                      \
                                  "  Info         : %s\n"                                                          \
                                  "  QvResult     : (not available)\n"                                                 \
                                  "  ErrorCode    : %d\n"                                                              \
                                  "  ErrorMessage : %s\n",                                                             \
                                  __FILE__, __LINE__, __func__, info, static_cast<int>(err_code), err_msg.c_str());          \
        }                                                                                                              \
    } while (0)

TEST(AttestationTestSuite, TestQuote) {
    int ret;
    RemoteAttestor attestor;
    std::string quote_report;
    std::string mrenclave_hex_in_report;
    ASSERT_FALSE(attestor.CreateReport("", quote_report));
    attestor.SetAcceptableResults({
        QvResult::Ok,
        QvResult::ConfigNeeded,
        QvResult::OutOfDate,
        QvResult::OutOfDateConfigNeeded,
        QvResult::SwHardeningNeeded,
        QvResult::ConfigAndSwHardeningNeeded
    });
    ASSERT_FALSE(attestor.VerifyReport("", quote_report, mrenclave_hex_in_report));
    PRINT_REMOTE_ATTESTOR_STATUS("ASSERT_FALSE", attestor);
    ASSERT_TRUE(ocall_verify_quote_untrusted(&ret, (uint8_t*)quote_report.c_str(), (int)quote_report.size(), 0, 0,
                                             "") == SGX_SUCCESS &&
                ret == -1);

    ASSERT_TRUE(attestor.CreateReport("1", quote_report));
    attestor.SetAcceptableResults({
        QvResult::Ok,
        QvResult::ConfigNeeded,
        QvResult::OutOfDate,
        QvResult::OutOfDateConfigNeeded,
        QvResult::SwHardeningNeeded,
        QvResult::ConfigAndSwHardeningNeeded
    });
    ASSERT_TRUE(attestor.VerifyReport("1", quote_report, mrenclave_hex_in_report));
    PRINT_REMOTE_ATTESTOR_STATUS("ASSERT_TRUE", attestor);
    ASSERT_EQ(mrenclave_hex_in_report.size(), 64);
    Printf("MRENCLAVE in report: %s\n", mrenclave_hex_in_report.c_str());
    ASSERT_TRUE(ocall_verify_quote_untrusted(&ret, (uint8_t*)quote_report.c_str(), (int)quote_report.size(), 0, 0,
                                             "1") == SGX_SUCCESS &&
                ret == 0);

    int64_t now = DateTime::Now().GetTimestamp();
    ASSERT_FALSE(attestor.CreateReport("", now, quote_report));
    attestor.SetAcceptableResults({
        QvResult::Ok,
        QvResult::ConfigNeeded,
        QvResult::OutOfDate,
        QvResult::OutOfDateConfigNeeded,
        QvResult::SwHardeningNeeded,
        QvResult::ConfigAndSwHardeningNeeded
    });
    ASSERT_FALSE(attestor.VerifyReport("", now, 3000, quote_report, mrenclave_hex_in_report));
    PRINT_REMOTE_ATTESTOR_STATUS("ASSERT_FALSE", attestor);
    ASSERT_TRUE(ocall_verify_quote_untrusted(&ret, (uint8_t*)quote_report.c_str(), (int)quote_report.size(), now, 3000,
                                             "") == SGX_SUCCESS &&
                ret == -1);

    ASSERT_TRUE(attestor.CreateReport("1", now, quote_report));
    attestor.SetAcceptableResults({
        QvResult::Ok,
        QvResult::ConfigNeeded,
        QvResult::OutOfDate,
        QvResult::OutOfDateConfigNeeded,
        QvResult::SwHardeningNeeded,
        QvResult::ConfigAndSwHardeningNeeded
    });
    ASSERT_TRUE(attestor.VerifyReport("1", now, 3000, quote_report, mrenclave_hex_in_report));
    PRINT_REMOTE_ATTESTOR_STATUS("ASSERT_TRUE", attestor);
    ASSERT_EQ(mrenclave_hex_in_report.size(), 64);
    Printf("MRENCLAVE in report: %s\n", mrenclave_hex_in_report.c_str());
    ASSERT_TRUE(ocall_verify_quote_untrusted(&ret, (uint8_t*)quote_report.c_str(), (int)quote_report.size(), now, 3000,
                                             "1") == SGX_SUCCESS &&
                ret == 0);

    uint8_t report_data[64] = {0};
    ASSERT_TRUE(attestor.CreateReport(report_data, quote_report));
    attestor.SetAcceptableResults({
        QvResult::Ok,
        QvResult::ConfigNeeded,
        QvResult::OutOfDate,
        QvResult::OutOfDateConfigNeeded,
        QvResult::SwHardeningNeeded,
        QvResult::ConfigAndSwHardeningNeeded
    });
    ASSERT_TRUE(attestor.VerifyReport(report_data, quote_report, mrenclave_hex_in_report));
    PRINT_REMOTE_ATTESTOR_STATUS("ASSERT_TRUE", attestor);
    ASSERT_EQ(mrenclave_hex_in_report.size(), 64);
    Printf("MRENCLAVE in report: %s\n", mrenclave_hex_in_report.c_str());
    ASSERT_TRUE(ocall_verify_quote_untrusted_original(&ret, (uint8_t*)quote_report.c_str(), (int)quote_report.size(),
                                                      report_data) == SGX_SUCCESS &&
                ret == 0);
}

TEST(AttestationTestSuite, TestQuoteUserInfo) {
    int ret;
    RemoteAttestor attestor;
    std::string quote_report;
    std::string mrenclave_hex_in_report;
    std::string user_info =
        "He was an old man who fished alone in a skiff in the Gulf Stream and he had gone eighty-four days now without "
        "taking a fish. In the first forty days a boy had been with him. But after forty days without a fish the boy’s "
        "parents had told him that the old man was now definitely and finally salao, which is the worst form of "
        "unlucky, "
        "and the boy had gone at their orders in another boat which caught three good fish the first week. It made the "
        "boy "
        "sad to see the old man come in each day with his skiff empty and he always went down to help him carry either "
        "the "
        "coiled lines or the gaff and harpoon and the sail that was furled around the mast. The sail was patched with "
        "flour "
        "sacks and, furled, it looked like the flag of permanent defeat.";
    ASSERT_TRUE(attestor.CreateReport(user_info, quote_report));
    attestor.SetAcceptableResults({
        QvResult::Ok,
        QvResult::ConfigNeeded,
        QvResult::OutOfDate,
        QvResult::OutOfDateConfigNeeded,
        QvResult::SwHardeningNeeded,
        QvResult::ConfigAndSwHardeningNeeded
    });
    ASSERT_FALSE(attestor.VerifyReport("123213213321", quote_report, mrenclave_hex_in_report));
    PRINT_REMOTE_ATTESTOR_STATUS("ASSERT_FALSE", attestor);
    attestor.SetAcceptableResults({
        QvResult::Ok,
        QvResult::ConfigNeeded,
        QvResult::OutOfDate,
        QvResult::OutOfDateConfigNeeded,
        QvResult::SwHardeningNeeded,
        QvResult::ConfigAndSwHardeningNeeded
    });
    ASSERT_TRUE(attestor.VerifyReport(user_info, quote_report, mrenclave_hex_in_report));
    PRINT_REMOTE_ATTESTOR_STATUS("ASSERT_TRUE", attestor);
    ASSERT_EQ(mrenclave_hex_in_report.size(), 64);
    Printf("MRENCLAVE in report: %s\n", mrenclave_hex_in_report.c_str());
    attestor.SetAcceptableResults({
        QvResult::Ok,
        QvResult::ConfigNeeded,
        QvResult::OutOfDate,
        QvResult::OutOfDateConfigNeeded,
        QvResult::SwHardeningNeeded,
        QvResult::ConfigAndSwHardeningNeeded
    });
    ASSERT_FALSE(attestor.VerifyReport("", quote_report, mrenclave_hex_in_report));
    PRINT_REMOTE_ATTESTOR_STATUS("ASSERT_FALSE", attestor);
    ASSERT_TRUE(ocall_verify_quote_untrusted(&ret, (uint8_t*)quote_report.c_str(), (int)quote_report.size(), 0, 0,
                                             "123213213321") == SGX_SUCCESS &&
                ret == -1);
    ASSERT_TRUE(ocall_verify_quote_untrusted(&ret, (uint8_t*)quote_report.c_str(), (int)quote_report.size(), 0, 0,
                                             user_info.c_str()) == SGX_SUCCESS &&
                ret == 0);
    ASSERT_TRUE(ocall_verify_quote_untrusted(&ret, (uint8_t*)quote_report.c_str(), (int)quote_report.size(), 0, 0,
                                             "") == SGX_SUCCESS &&
                ret == -1);
    uint8_t report_data[64] = {0};
    CSHA256 sha256;
    sha256.Write((uint8_t*)user_info.c_str(), user_info.size());
    sha256.Finalize(report_data);
    attestor.SetAcceptableResults({
        QvResult::Ok,
        QvResult::ConfigNeeded,
        QvResult::OutOfDate,
        QvResult::OutOfDateConfigNeeded,
        QvResult::SwHardeningNeeded,
        QvResult::ConfigAndSwHardeningNeeded
    });
    ASSERT_TRUE(attestor.VerifyReport(report_data, quote_report, mrenclave_hex_in_report));
    PRINT_REMOTE_ATTESTOR_STATUS("ASSERT_TRUE", attestor);
    ASSERT_EQ(mrenclave_hex_in_report.size(), 64);
    Printf("MRENCLAVE in report: %s\n", mrenclave_hex_in_report.c_str());
    ASSERT_TRUE(ocall_verify_quote_untrusted_original(&ret, (uint8_t*)quote_report.c_str(), (int)quote_report.size(),
                                                      report_data) == SGX_SUCCESS &&
                ret == 0);

    int64_t now = DateTime::Now().GetTimestamp();
    ASSERT_TRUE(attestor.CreateReport(user_info, now, quote_report));
    attestor.SetAcceptableResults({
        QvResult::Ok,
        QvResult::ConfigNeeded,
        QvResult::OutOfDate,
        QvResult::OutOfDateConfigNeeded,
        QvResult::SwHardeningNeeded,
        QvResult::ConfigAndSwHardeningNeeded
    });
    ASSERT_FALSE(attestor.VerifyReport("123213213321", now, 3000, quote_report, mrenclave_hex_in_report));
    PRINT_REMOTE_ATTESTOR_STATUS("ASSERT_FALSE", attestor);
    attestor.SetAcceptableResults({
        QvResult::Ok,
        QvResult::ConfigNeeded,
        QvResult::OutOfDate,
        QvResult::OutOfDateConfigNeeded,
        QvResult::SwHardeningNeeded,
        QvResult::ConfigAndSwHardeningNeeded
    });
    ASSERT_TRUE(attestor.VerifyReport(user_info, now, 3000, quote_report, mrenclave_hex_in_report));
    PRINT_REMOTE_ATTESTOR_STATUS("ASSERT_TRUE", attestor);
    ASSERT_EQ(mrenclave_hex_in_report.size(), 64);
    Printf("MRENCLAVE in report: %s\n", mrenclave_hex_in_report.c_str());
    attestor.SetAcceptableResults({
        QvResult::Ok,
        QvResult::ConfigNeeded,
        QvResult::OutOfDate,
        QvResult::OutOfDateConfigNeeded,
        QvResult::SwHardeningNeeded,
        QvResult::ConfigAndSwHardeningNeeded
    });
    ASSERT_FALSE(attestor.VerifyReport("", now, 3000, quote_report, mrenclave_hex_in_report));
    PRINT_REMOTE_ATTESTOR_STATUS("ASSERT_FALSE", attestor);
    ASSERT_TRUE(ocall_verify_quote_untrusted(&ret, (uint8_t*)quote_report.c_str(), (int)quote_report.size(), now, 3000,
                                             "123213213321") == SGX_SUCCESS &&
                ret == -1);
    ASSERT_TRUE(ocall_verify_quote_untrusted(&ret, (uint8_t*)quote_report.c_str(), (int)quote_report.size(), now, 3000,
                                             user_info.c_str()) == SGX_SUCCESS &&
                ret == 0);
    ASSERT_TRUE(ocall_verify_quote_untrusted(&ret, (uint8_t*)quote_report.c_str(), (int)quote_report.size(), now, 3000,
                                             "") == SGX_SUCCESS &&
                ret == -1);
}

TEST(AttestationTestSuite, TestUserData) {
    int ret;
    std::string mrenclave_hex_in_report;
    RemoteAttestor attestor;
    std::string quote_report_base64 =
        "AwACAAAAAAALABAAk5pyM/ecTKmUCg2zlX8GB/5uFiIpSJzDeqgvnzkPBNIAAAAADAwQD///AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
        "AAAAAAAAAAAAAAAAAAAAAABQAAAAAAAADnAAAAAAAAAENV/AaCJsBmSSOR26mSc3DfEiFCNBfTwF8zfitVkwf2AAAAAAAAAAAAAAAAAAAA"
        "AAAAAAAAAAAAAAAAAAAAAAAJNqo/ZDz8KW39byaZr66+F+N3GdGcG4/br1ZLgjdgvAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAyhAAAHsWPyaBA0kyLjNgxehVHTQOFdGE3veMuWaWONsb37BmnkR9eMcLRQkc"
        "W9d1JK4cr0Nl3VIrXKWByQf9X2aJsyUQK31eEmuOPsnYNbcklXkZd9JDCEYBJkGo/fLXF0+PyjT3WMbUDFzWOTvwlLfNniz6PoxzFJvxwK"
        "xb8bupvk5oDAwQD///AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAFQAAAAAAAADnAAAAAAAAAHj+jP0BCVoP"
        "EIr/XEBiS5NhLWwotz4ajSgXnJ3fDgaGAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAACMT1d115ZQPpYTf3fGioKaAFasje1wFA"
        "sIGwlEkMV7/wAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAEACwAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
        "AAAAAAAAAAAAAVVAwAN9/DtZ3JJfK4/p1yiw3zI20PAgShnMTp41QkUwAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAARTqpHK7"
        "v8rtmwbIIU/3Rz+Ux09NyoJwlbFeGYfJex7yuhDRwZQF6OmukFtYdRtGlLylN1Nl9ZXx7ra/ex018CAAAAECAwQFBgcICQoLDA0ODxAREh"
        "MUFRYXGBkaGxwdHh8FAGIOAAAtLS0tLUJFR0lOIENFUlRJRklDQVRFLS0tLS0KTUlJRTh6Q0NCSm1nQXdJQkFnSVZBSTVUa0tmTEIwUkZX"
        "MmhKWERlV25DT2RCUnZLTUFvR0NDcUdTTTQ5QkFNQwpNSEF4SWpBZ0JnTlZCQU1NR1VsdWRHVnNJRk5IV0NCUVEwc2dVR3hoZEdadmNtMG"
        "dRMEV4R2pBWUJnTlZCQW9NCkVVbHVkR1ZzSUVOdmNuQnZjbUYwYVc5dU1SUXdFZ1lEVlFRSERBdFRZVzUwWVNCRGJHRnlZVEVMTUFrR0Ex"
        "VUUKQ0F3Q1EwRXhDekFKQmdOVkJBWVRBbFZUTUI0WERUSTFNREl5TWpFMk1ESXlNVm9YRFRNeU1ESXlNakUyTURJeQpNVm93Y0RFaU1DQU"
        "dBMVVFQXd3WlNXNTBaV3dnVTBkWUlGQkRTeUJEWlhKMGFXWnBZMkYwWlRFYU1CZ0dBMVVFCkNnd1JTVzUwWld3Z1EyOXljRzl5WVhScGIy"
        "NHhGREFTQmdOVkJBY01DMU5oYm5SaElFTnNZWEpoTVFzd0NRWUQKVlFRSURBSkRRVEVMTUFrR0ExVUVCaE1DVlZNd1dUQVRCZ2NxaGtqT1"
        "BRSUJCZ2dxaGtqT1BRTUJCd05DQUFSMwpQSFB6VEN0NldEVE9xNjdXQ2UvRzk4MEYzVDJxaktHRjJUTjNFMG9Id2VSenhuNnMxa2JSVnFJ"
        "WFN6ZW8zcWRLCkVGT0YrQ3hIcEJ6T0t0dzIvUDVIbzRJRERqQ0NBd293SHdZRFZSMGpCQmd3Rm9BVWxXOWR6YjBiNGVsQVNjblUKOURQT0"
        "FWY0wzbFF3YXdZRFZSMGZCR1F3WWpCZ29GNmdYSVphYUhSMGNITTZMeTloY0drdWRISjFjM1JsWkhObApjblpwWTJWekxtbHVkR1ZzTG1O"
        "dmJTOXpaM2d2WTJWeWRHbG1hV05oZEdsdmJpOTJNeTl3WTJ0amNtdy9ZMkU5CmNHeGhkR1p2Y20wbVpXNWpiMlJwYm1jOVpHVnlNQjBHQT"
        "FVZERnUVdCQlNNSTlIN2RCekFORkZIeTdnWmlRYSsKNGY5VktEQU9CZ05WSFE4QkFmOEVCQU1DQnNBd0RBWURWUjBUQVFIL0JBSXdBREND"
        "QWpzR0NTcUdTSWI0VFFFTgpBUVNDQWl3d2dnSW9NQjRHQ2lxR1NJYjRUUUVOQVFFRUVIMG5PckM1VElxTW5pRk5vcDJDdkFvd2dnRmxCZ2"
        "9xCmhraUcrRTBCRFFFQ01JSUJWVEFRQmdzcWhraUcrRTBCRFFFQ0FRSUJEREFRQmdzcWhraUcrRTBCRFFFQ0FnSUIKRERBUUJnc3Foa2lH"
        "K0UwQkRRRUNBd0lCQXpBUUJnc3Foa2lHK0UwQkRRRUNCQUlCQXpBUkJnc3Foa2lHK0UwQgpEUUVDQlFJQ0FQOHdFUVlMS29aSWh2aE5BUT"
        "BCQWdZQ0FnRC9NQkFHQ3lxR1NJYjRUUUVOQVFJSEFnRUFNQkFHCkN5cUdTSWI0VFFFTkFRSUlBZ0VBTUJBR0N5cUdTSWI0VFFFTkFRSUpB"
        "Z0VBTUJBR0N5cUdTSWI0VFFFTkFRSUsKQWdFQU1CQUdDeXFHU0liNFRRRU5BUUlMQWdFQU1CQUdDeXFHU0liNFRRRU5BUUlNQWdFQU1CQU"
        "dDeXFHU0liNApUUUVOQVFJTkFnRUFNQkFHQ3lxR1NJYjRUUUVOQVFJT0FnRUFNQkFHQ3lxR1NJYjRUUUVOQVFJUEFnRUFNQkFHCkN5cUdT"
        "SWI0VFFFTkFRSVFBZ0VBTUJBR0N5cUdTSWI0VFFFTkFRSVJBZ0VOTUI4R0N5cUdTSWI0VFFFTkFRSVMKQkJBTURBTUQvLzhBQUFBQUFBQU"
        "FBQUFBTUJBR0NpcUdTSWI0VFFFTkFRTUVBZ0FBTUJRR0NpcUdTSWI0VFFFTgpBUVFFQmdCZ2FnQUFBREFQQmdvcWhraUcrRTBCRFFFRkNn"
        "RUJNQjRHQ2lxR1NJYjRUUUVOQVFZRUVOSHNYa1lNCmp2YVltNU84Y0hoNjJHTXdSQVlLS29aSWh2aE5BUTBCQnpBMk1CQUdDeXFHU0liNF"
        "RRRU5BUWNCQVFIL01CQUcKQ3lxR1NJYjRUUUVOQVFjQ0FRSC9NQkFHQ3lxR1NJYjRUUUVOQVFjREFRSC9NQW9HQ0NxR1NNNDlCQU1DQTBn"
        "QQpNRVVDSVFDcXBOQWJoTzFJK0U0M1ZKYk1WUVdybGdxV1lOS3Q4amlmdHAvRS9DOStzZ0lnU1M1K1VNMjFOeVBFCll1RFVOMDJCWDk3c0"
        "ExakRuWVRMcVhrUzBCVmc5SEE9Ci0tLS0tRU5EIENFUlRJRklDQVRFLS0tLS0KLS0tLS1CRUdJTiBDRVJUSUZJQ0FURS0tLS0tCk1JSUNs"
        "akNDQWoyZ0F3SUJBZ0lWQUpWdlhjMjlHK0hwUUVuSjFQUXp6Z0ZYQzk1VU1Bb0dDQ3FHU000OUJBTUMKTUdneEdqQVlCZ05WQkFNTUVVbH"
        "VkR1ZzSUZOSFdDQlNiMjkwSUVOQk1Sb3dHQVlEVlFRS0RCRkpiblJsYkNCRApiM0p3YjNKaGRHbHZiakVVTUJJR0ExVUVCd3dMVTJGdWRH"
        "RWdRMnhoY21FeEN6QUpCZ05WQkFnTUFrTkJNUXN3CkNRWURWUVFHRXdKVlV6QWVGdzB4T0RBMU1qRXhNRFV3TVRCYUZ3MHpNekExTWpFeE"
        "1EVXdNVEJhTUhBeElqQWcKQmdOVkJBTU1HVWx1ZEdWc0lGTkhXQ0JRUTBzZ1VHeGhkR1p2Y20wZ1EwRXhHakFZQmdOVkJBb01FVWx1ZEdW"
        "cwpJRU52Y25CdmNtRjBhVzl1TVJRd0VnWURWUVFIREF0VFlXNTBZU0JEYkdGeVlURUxNQWtHQTFVRUNBd0NRMEV4CkN6QUpCZ05WQkFZVE"
        "FsVlRNRmt3RXdZSEtvWkl6ajBDQVFZSUtvWkl6ajBEQVFjRFFnQUVOU0IvN3QyMWxYU08KMkN1enB4dzc0ZUpCNzJFeURHZ1c1clhDdHgy"
        "dFZUTHE2aEtrNnorVWlSWkNucVI3cHNPdmdxRmVTeGxtVGxKbAplVG1pMldZejNxT0J1ekNCdURBZkJnTlZIU01FR0RBV2dCUWlaUXpXV3"
        "AwMGlmT0R0SlZTdjFBYk9TY0dyREJTCkJnTlZIUjhFU3pCSk1FZWdSYUJEaGtGb2RIUndjem92TDJObGNuUnBabWxqWVhSbGN5NTBjblZ6"
        "ZEdWa2MyVnkKZG1salpYTXVhVzUwWld3dVkyOXRMMGx1ZEdWc1UwZFlVbTl2ZEVOQkxtUmxjakFkQmdOVkhRNEVGZ1FVbFc5ZAp6YjBiNG"
        "VsQVNjblU5RFBPQVZjTDNsUXdEZ1lEVlIwUEFRSC9CQVFEQWdFR01CSUdBMVVkRXdFQi93UUlNQVlCCkFmOENBUUF3Q2dZSUtvWkl6ajBF"
        "QXdJRFJ3QXdSQUlnWHNWa2kwdytpNlZZR1czVUYvMjJ1YVhlMFlKRGoxVWUKbkErVGpEMWFpNWNDSUNZYjFTQW1ENXhrZlRWcHZvNFVveW"
        "lTWXhyRFdMbVVSNENJOU5LeWZQTisKLS0tLS1FTkQgQ0VSVElGSUNBVEUtLS0tLQotLS0tLUJFR0lOIENFUlRJRklDQVRFLS0tLS0KTUlJ"
        "Q2p6Q0NBalNnQXdJQkFnSVVJbVVNMWxxZE5JbnpnN1NWVXI5UUd6a25CcXd3Q2dZSUtvWkl6ajBFQXdJdwphREVhTUJnR0ExVUVBd3dSU1"
        "c1MFpXd2dVMGRZSUZKdmIzUWdRMEV4R2pBWUJnTlZCQW9NRVVsdWRHVnNJRU52CmNuQnZjbUYwYVc5dU1SUXdFZ1lEVlFRSERBdFRZVzUw"
        "WVNCRGJHRnlZVEVMTUFrR0ExVUVDQXdDUTBFeEN6QUoKQmdOVkJBWVRBbFZUTUI0WERURTRNRFV5TVRFd05EVXhNRm9YRFRRNU1USXpNVE"
        "l6TlRrMU9Wb3dhREVhTUJnRwpBMVVFQXd3UlNXNTBaV3dnVTBkWUlGSnZiM1FnUTBFeEdqQVlCZ05WQkFvTUVVbHVkR1ZzSUVOdmNuQnZj"
        "bUYwCmFXOXVNUlF3RWdZRFZRUUhEQXRUWVc1MFlTQkRiR0Z5WVRFTE1Ba0dBMVVFQ0F3Q1EwRXhDekFKQmdOVkJBWVQKQWxWVE1Ga3dFd1"
        "lIS29aSXpqMENBUVlJS29aSXpqMERBUWNEUWdBRUM2bkV3TURJWVpPai9pUFdzQ3phRUtpNwoxT2lPU0xSRmhXR2pibkJWSmZWbmtZNHUz"
        "SWprRFlZTDBNeE80bXFzeVlqbEJhbFRWWXhGUDJzSkJLNXpsS09CCnV6Q0J1REFmQmdOVkhTTUVHREFXZ0JRaVpReldXcDAwaWZPRHRKVl"
        "N2MUFiT1NjR3JEQlNCZ05WSFI4RVN6QkoKTUVlZ1JhQkRoa0ZvZEhSd2N6b3ZMMk5sY25ScFptbGpZWFJsY3k1MGNuVnpkR1ZrYzJWeWRt"
        "bGpaWE11YVc1MApaV3d1WTI5dEwwbHVkR1ZzVTBkWVVtOXZkRU5CTG1SbGNqQWRCZ05WSFE0RUZnUVVJbVVNMWxxZE5JbnpnN1NWClVyOV"
        "FHemtuQnF3d0RnWURWUjBQQVFIL0JBUURBZ0VHTUJJR0ExVWRFd0VCL3dRSU1BWUJBZjhDQVFFd0NnWUkKS29aSXpqMEVBd0lEU1FBd1Jn"
        "SWhBT1cvNVFrUitTOUNpU0RjTm9vd0x1UFJMc1dHZi9ZaTdHU1g5NEJnd1R3ZwpBaUVBNEowbHJIb01zK1hvNW8vc1g2TzlRV3hIUkF2Wl"
        "VHT2RSUTdjdnFSWGFxST0KLS0tLS1FTkQgQ0VSVElGSUNBVEUtLS0tLQoA";

    uint8_t report_data[64] = {0};
    attestor.SetAcceptableResults({
        QvResult::Ok,
        QvResult::ConfigNeeded,
        QvResult::OutOfDate,
        QvResult::OutOfDateConfigNeeded,
        QvResult::SwHardeningNeeded,
        QvResult::ConfigAndSwHardeningNeeded
    });
    ASSERT_FALSE(attestor.VerifyReport("", quote_report_base64, mrenclave_hex_in_report));
    PRINT_REMOTE_ATTESTOR_STATUS("ASSERT_FALSE", attestor);
    attestor.SetAcceptableResults({
        QvResult::Ok,
        QvResult::ConfigNeeded,
        QvResult::OutOfDate,
        QvResult::OutOfDateConfigNeeded,
        QvResult::SwHardeningNeeded,
        QvResult::ConfigAndSwHardeningNeeded
    });
    ASSERT_TRUE(attestor.VerifyReport(report_data, quote_report_base64, mrenclave_hex_in_report));
    PRINT_REMOTE_ATTESTOR_STATUS("ASSERT_TRUE", attestor);
    ASSERT_EQ(mrenclave_hex_in_report.size(), 64);
    Printf("MRENCLAVE in report: %s\n", mrenclave_hex_in_report.c_str());
    ASSERT_TRUE(ocall_verify_quote_untrusted_original(&ret, (uint8_t*)quote_report_base64.c_str(),
                                                      (int)quote_report_base64.size(), report_data) == SGX_SUCCESS &&
                ret == 0);

    std::string quote_report_bytes = safeheron::encode::base64::DecodeFromBase64(quote_report_base64);
    attestor.SetAcceptableResults({
        QvResult::Ok,
        QvResult::ConfigNeeded,
        QvResult::OutOfDate,
        QvResult::OutOfDateConfigNeeded,
        QvResult::SwHardeningNeeded,
        QvResult::ConfigAndSwHardeningNeeded
    });
    ASSERT_FALSE(attestor.VerifyReport(report_data, quote_report_bytes, mrenclave_hex_in_report));
}

TEST(AttestationTestSuite, TestTimestamp) {
    int ret;
    std::string mrenclave_hex_in_report;
    RemoteAttestor attestor;
    std::string quote_report;
    int64_t now = DateTime::Now().GetTimestamp();
    ASSERT_TRUE(attestor.CreateReport("1", now, quote_report));
    Sleep(3);
    attestor.SetAcceptableResults({
        QvResult::Ok,
        QvResult::ConfigNeeded,
        QvResult::OutOfDate,
        QvResult::OutOfDateConfigNeeded,
        QvResult::SwHardeningNeeded,
        QvResult::ConfigAndSwHardeningNeeded
    });
    ASSERT_FALSE(attestor.VerifyReport("1", now, 1, quote_report, mrenclave_hex_in_report));
    PRINT_REMOTE_ATTESTOR_STATUS("ASSERT_FALSE", attestor);
    ASSERT_TRUE(ocall_verify_quote_untrusted(&ret, (uint8_t*)quote_report.c_str(), (int)quote_report.size(), now, 1,
                                             "1") == SGX_SUCCESS &&
                ret == -1);
    attestor.SetAcceptableResults({
        QvResult::Ok,
        QvResult::ConfigNeeded,
        QvResult::OutOfDate,
        QvResult::OutOfDateConfigNeeded,
        QvResult::SwHardeningNeeded,
        QvResult::ConfigAndSwHardeningNeeded
    });
    ASSERT_TRUE(attestor.VerifyReport("1", now, 300, quote_report, mrenclave_hex_in_report));
    PRINT_REMOTE_ATTESTOR_STATUS("ASSERT_TRUE", attestor);
    ASSERT_EQ(mrenclave_hex_in_report.size(), 64);
    Printf("MRENCLAVE in report: %s\n", mrenclave_hex_in_report.c_str());
    ASSERT_TRUE(ocall_verify_quote_untrusted(&ret, (uint8_t*)quote_report.c_str(), (int)quote_report.size(), now, 300,
                                             "1") == SGX_SUCCESS &&
                ret == 0);
}
