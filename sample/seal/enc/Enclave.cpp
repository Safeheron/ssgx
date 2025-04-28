#include <ctime>
#include <vector>

#include "sgx_tseal.h"

#include "ssgx_filesystem_t.h"
#include "ssgx_utils_t.h"

#include "Enclave_t.h"

using ssgx::filesystem_t::FileMode;
using ssgx::filesystem_t::Path;
using ssgx::filesystem_t::ProtectedFileReader;
using ssgx::filesystem_t::ProtectedFileWriter;
using ssgx::utils_t::SealHandler;

/**
 * Encryption and decryption using the MRENCLAVE method
 * @return
 */
int seal_sample1() {
    ssgx::utils_t::Printf("Sample 1 start\n");
    SealHandler sealer(SGX_KEYPOLICY_MRENCLAVE);
    uint8_t mac_text[] = {0xAA, 0xBB, 0xCC, 0xDD};
    sealer.SetAdditionalMacText(mac_text, sizeof(mac_text));
    std::string raw_data = "MACProtectedData";
    auto sealed_data = sealer.SealData(reinterpret_cast<const uint8_t*>(raw_data.c_str()), raw_data.size());

    auto unsealed_data = sealer.UnsealData(sealed_data->data(), sealed_data->size());
    if (std::string(unsealed_data->decrypted_text.begin(), unsealed_data->decrypted_text.end()) == "MACProtectedData") {
        ssgx::utils_t::Printf("Sample 1 execution successfully.\n");
    } else {
        ssgx::utils_t::Printf("Sample 1 execution failed.\n");
    }
    return 0;
}
/**
 * Encryption and decryption using the MRENCLAVE + MRSIGNER method
 * @return
 */
int seal_sample2() {
    ssgx::utils_t::Printf("Sample 2 start\n");
    SealHandler sealer(SGX_KEYPOLICY_MRENCLAVE | SGX_KEYPOLICY_MRSIGNER);
    uint8_t mac_text[] = {0xAA, 0xBB, 0xCC, 0xDD};
    sealer.SetAdditionalMacText(mac_text, sizeof(mac_text));

    const char* raw_data = "MACProtectedData";
    size_t data_len = strlen(raw_data);
    auto sealed_data = sealer.SealData(reinterpret_cast<const uint8_t*>(raw_data), data_len);
    auto unsealed_data = sealer.UnsealData(sealed_data->data(), sealed_data->size());
    if (std::string(unsealed_data->decrypted_text.begin(), unsealed_data->decrypted_text.end()) == "MACProtectedData") {
        ssgx::utils_t::Printf("Sample 2 execution successfully.\n");
    } else {
        ssgx::utils_t::Printf("Sample 2 execution failed.\n");
    }
    return 0;
}
/**
 * Encrypt and save to a file using the MRENCLAVE and MRSIGNER method.
 * @return
 */
int seal_sample3() {
    ssgx::utils_t::Printf("Sample 3 start\n");
    constexpr uint32_t SIZE_PER_TIME = 4 * 1024; // 4KB each time
    std::string content = "The is a protected text file! File content is sealed by seal key.";
    Path file_name = Path("test_file_name");
    uint16_t key_policy = (SGX_KEYPOLICY_MRSIGNER | SGX_KEYPOLICY_MRENCLAVE);

    size_t written_size = 0;
    size_t left_size = content.size();
    try {
        ProtectedFileWriter writer(file_name.String().c_str(), FileMode::OpenOrCreate, key_policy);
        while (left_size > 0) {
            size_t write_size = (left_size > SIZE_PER_TIME) ? SIZE_PER_TIME : left_size;
            writer.Write(content.c_str() + written_size, write_size);
            left_size -= write_size;
        }
        writer.Close();
    } catch (...) {
        ssgx::utils_t::Printf("Sample 3 execution failed.\n");
    }

    ssgx::utils_t::Printf("Sample 3 execution successfully.\n");

    return 0;
}
/**
 *  MRENCLAVE + MRSIGNER file reading and decryption (no need to specify policy, as the file already contains the
 * encryption policy)
 * @return
 */
int seal_sample4() {
    ssgx::utils_t::Printf("Sample 4 start\n");
    Path file_name = Path("test_file_name");
    std::string content = "The is a protected text file! File content is sealed by seal key.";
    constexpr uint32_t SIZE_PER_TIME = 4 * 1024; // 4KB each time
    uint8_t buffer[SIZE_PER_TIME] = {0};

    size_t read_size = 0;
    std::string test_data;
    try {
        ProtectedFileReader reader(file_name.String().c_str());
        while ((read_size = reader.Read(buffer, SIZE_PER_TIME)) > 0) {
            test_data.append((char*)buffer, read_size);
        }
        reader.Close();
    } catch (...) {
        ssgx::utils_t::Printf("Sample 4 execution failed.\n");
    }
    if (content == test_data) {
        ssgx::utils_t::Printf("Sample 4 execution successfully.\n");
    } else {
        ssgx::utils_t::Printf("Sample 4 execution failed.\n");
    }

    return 0;
}
int ecall_run_test() {
    seal_sample1();
    seal_sample2();
    seal_sample3();
    seal_sample4();
    return 0;
}
