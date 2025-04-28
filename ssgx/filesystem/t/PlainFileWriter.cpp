#include "sgx_lfence.h"
#include "sgx_tprotected_fs.h"
#include "sgx_trts.h"
#include "sgx_utils.h"

#include "ssgx_filesystem_t.h"
#include "ssgx_filesystem_t_t.h"
#include "ssgx_utils_t.h"

#include "../../common/internal_check.h"
#include "FileMetaData.h"
#include "filesystem_constant.h"

namespace ssgx {
namespace filesystem_t {

// Write data to a plain file from outside of enclave
static void WritePlainFile(const std::string& filepath, bool is_binary, const uint8_t* data, size_t size) {
    int ret = 0;
    sgx_status_t sgx_status = SGX_SUCCESS;

    if (filepath.empty()) {
        throw FileSystemException("Invalid parameter, filepath is empty");
    }

    if (size > FS_MAX_FILE_SIZE) {
        throw FileSystemException("Invalid parameter, data size has exceeded the max (100 KB)");
    }

    sgx_status = ssgx_ocall_write_file(&ret, filepath.c_str(), is_binary ? 1 : 0, data, size);
    if (sgx_status != SGX_SUCCESS) {
        throw FileSystemException(
            ssgx::utils_t::FormatStr("Failed to call function ssgx_ocall_write_file(), sgx status: 0x%x", sgx_status));
    }
    if (ret < 0) {
        throw FileSystemException(
            ssgx::utils_t::FormatStr("Function ssgx_ocall_write_file() return errors, error code: %d", ret));
    }
}

void PlainFileWriter::WriteAllBytes(const std::vector<uint8_t>& data) {
    WritePlainFile(file_path_, true, data.data(), data.size());
}

void PlainFileWriter::WriteAllText(const std::string& str) {
    WritePlainFile(file_path_, false, reinterpret_cast<const uint8_t*>(str.c_str()), str.length());
}

} // namespace filesystem_t
} // namespace ssgx