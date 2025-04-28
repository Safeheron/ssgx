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

ProtectedFileReader::ProtectedFileReader(const char* file_name) : file_(nullptr), file_name_(file_name) {
    sgx_status_t status = SGX_SUCCESS;

    if (!file_name) {
        throw FileSystemException("File name cannot be null");
    }

    // at first, try to read metadata file and throw exception if failed.
    Path metadata_file_path(file_name);
    metadata_file_path += FS_METADATA_FILE_EXT;
    std::optional<FileMetaData> meta_data = FileMetaData::FromFile(metadata_file_path.String());

    // If metadata file exists, but failed to load it
    if (!meta_data.has_value() && Exists(metadata_file_path)) {
        throw FileSystemException("Metadata file is invalid.");
    }

    // If failed to read the metadata file, or legacy_mode is 1 in the metadata file,
    // try to use sgx_fopen_auto_key() to open the protected file, because perhaps it is a legacy file
    // created by other applications.
    if (!meta_data.has_value() || meta_data.value().GetLegacyMode() == 1) {
        file_ = sgx_fopen_auto_key(file_name, "r");
        if (!file_) {
            throw FileSystemException("Failed to open the file in auto key mode.");
        }
    }
    // If the metadata file is read successfully, use the key_request data in it
    // and call sgx_fopen() to open the protected file.
    else {
        // get key_request from metadata and use it to get seal key
        sgx_key_128bit_t seal_key = {0};
        sgx_key_request_t key_request = meta_data.value().GetKeyRequest();
        status = sgx_get_key(&key_request, &seal_key);
        if (status != SGX_SUCCESS) {
            throw FileSystemException("Failed to get seal key");
        }

        // open file for reading
        file_ = sgx_fopen(file_name, "r", &seal_key);
        if (!file_) {
            memset_s(seal_key, sizeof(sgx_key_128bit_t), 0, sizeof(sgx_key_128bit_t));
            throw FileSystemException("Failed to open the file for reading");
        }

        // clear key data
        memset_s(seal_key, sizeof(sgx_key_128bit_t), 0, sizeof(sgx_key_128bit_t));
    }
}

ProtectedFileReader::~ProtectedFileReader() {
    if (file_) {
        sgx_fclose(file_);
    }
}

size_t ProtectedFileReader::Read(void* buffer, size_t size) const {
    if (!file_ || !buffer || size == 0) {
        throw FileSystemException("Invalid read parameters");
    }
    size_t bytes_read = sgx_fread(buffer, 1, size, file_);
    if (bytes_read == 0 && sgx_ferror(file_) != 0) {
        throw FileSystemException("Read error");
    }
    return bytes_read;
}

int64_t ProtectedFileReader::Tell() const {
    if (!file_) {
        throw FileSystemException("File not open");
    }
    const int64_t pos = sgx_ftell(file_);
    if (pos < 0) {
        throw FileSystemException("Tell error");
    }
    return pos;
}

void ProtectedFileReader::Seek(int64_t offset, int origin) const {
    if (!file_) {
        throw FileSystemException("File not open");
    }
    if (sgx_fseek(file_, offset, origin) != 0) {
        throw FileSystemException("Seek error");
    }
}

void ProtectedFileReader::Close() {
    if (file_) {
        if (sgx_fclose(file_) != 0) {
            throw FileSystemException("Close error");
        }
        file_ = nullptr;
    }
}

} // namespace filesystem_t
} // namespace ssgx