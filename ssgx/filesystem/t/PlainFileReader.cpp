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

// Read a plain file from outside of enclave
static size_t ReadPlainFile(const std::string& filepath, bool is_binary, uint8_t** buffer) {
    int ret = 0;
    size_t data_size = 0;
    uint8_t* data_buf = nullptr;
    sgx_status_t sgx_status = SGX_SUCCESS;

    *buffer = nullptr;

    if (filepath.empty()) {
        throw FileSystemException("Invalid parameter, filepath is empty");
    }

    // read file data by ocall
    sgx_status = ssgx_ocall_read_file(&ret, filepath.c_str(), is_binary ? 1 : 0, &data_buf, &data_size);
    if (sgx_status != SGX_SUCCESS) {
        throw FileSystemException(
            ssgx::utils_t::FormatStr("Failed to call function ssgx_ocall_read_file(), sgx status: 0x%x", sgx_status));
    }
    if (ret < 0) {
        throw FileSystemException(
            ssgx::utils_t::FormatStr("Function ssgx_ocall_read_file() return errors, error code: %d", ret));
    }

    // if data_size > 0, assign data_buf to output buffer pointer
    if (data_size > 0) {
        // encounter an exception!
        if (!data_buf) {
            throw FileSystemException("Internal error, data_buf is nullptr");
        }
        if (sgx_is_outside_enclave(data_buf, data_size) == 0) {
            // Don't call ssgx::utils_t::FreeOutside(file_buf, file_size); in this case
            throw FileSystemException("Invalid external input detected, with traces of enclave memory found");
        }
        sgx_lfence();

        // return read data and free buffer
        *buffer = data_buf;
    }

    return data_size;
}

std::vector<uint8_t> PlainFileReader::ReadAllBytes() const {
    std::vector<uint8_t> result;
    uint8_t* data_buffer = nullptr;
    const size_t read_size = ReadPlainFile(file_path_, true, &data_buffer);
    if (data_buffer) {
        result = std::vector<uint8_t>(data_buffer, data_buffer + read_size);
        utils_t::FreeOutside(data_buffer, read_size);
        data_buffer = nullptr;
    }
    return result;
}

std::string PlainFileReader::ReadAllText() const {
    std::string result;
    uint8_t* data_buffer = nullptr;
    const size_t read_size = ReadPlainFile(file_path_, false, &data_buffer);
    if (data_buffer) {
        result.assign(data_buffer, data_buffer + read_size);
        utils_t::FreeOutside(data_buffer, read_size);
        data_buffer = nullptr;
    }
    return result;
}

} // namespace filesystem_t
} // namespace ssgx