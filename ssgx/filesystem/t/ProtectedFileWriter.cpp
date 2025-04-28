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
namespace detail {
/**
 * @brief Open or create a file based on key_policy for writing.
 *      If the file exists, the file will be overwritten,
 *      otherwise a new file will be created.
 *
 * @param file_name the protected file name
 * @param meta_file_name the metadata file name
 * @param key_policy seal key policy, if SGX_KEYPOLICY_MRSIGNER is specified, will call sgx_fopen_auto_key() to
 *      create or open the file.
 * @return the file handler
 *
 * @throw throw FileSystemException if failed
 */
static SGX_FILE* OpenFileForWrite(const char* file_name, const char* meta_file_name, uint16_t key_policy) {
    // Get enclave self report
    const sgx_report_t* report = sgx_self_report();
    if (!report) {
        throw FileSystemException("Failed to get self Enclave Report");
    }

    // Currently, set legacy_mode = 1 when key_policy is SGX_KEYPOLICY_MRSIGNER.
    // In this case, call API sgx_fopen_auto_key() to create or open the file, so that the file
    // can be opened by other applications which use the same API.
    FileMetaData metadata;
    SGX_FILE* file = nullptr;
    if (key_policy == SGX_KEYPOLICY_MRSIGNER) {
        // key_id will be ignored in this case, so use 0 as the key_id.
        sgx_key_id_t key_id = {0};
        metadata = FileMetaData(1, key_policy, key_id, report->body.isv_svn, report->body.cpu_svn);

        // Call sgx_fopen_auto_key() to create or open the file.
        // In this API, a key will be derived with SGX_KEYPOLICY_MRSIGNER
        file = sgx_fopen_auto_key(file_name, "w");
    }
    else {
        // Use a random number for key_id
        sgx_key_id_t key_id = {0};
        sgx_status_t status = sgx_read_rand(key_id.id, sizeof(key_id));
        if (status != SGX_SUCCESS) {
            throw FileSystemException("Failed to read random number generator");
        }
        metadata = FileMetaData(0, key_policy, key_id, report->body.isv_svn, report->body.cpu_svn);
        sgx_key_request_t key_request = metadata.GetKeyRequest();

        // Get a seal key
        sgx_key_128bit_t seal_key = {0};
        status = sgx_get_key(&key_request, &seal_key);
        if (status != SGX_SUCCESS) {
            throw FileSystemException("Failed to get seal key");
        }

        // Call sgx_fopen() to create or open the file
        file = sgx_fopen(file_name, "w", &seal_key);

        // Reset seal_key with 0
        memset_s(seal_key, sizeof(sgx_key_128bit_t), 0, sizeof(sgx_key_128bit_t));
    }
    if (!file) {
        throw FileSystemException("Failed to open the file for writing");
    }

    if (!metadata.ToFile(meta_file_name)) {
        sgx_fclose(file);
        Remove(Path(file_name));
        throw FileSystemException("Failed to create metadata file");
    }

    return file;
}

/**
 * @brief Open or create a file based on key_policy for appending.
 *      If the file exists, seeks to the end of the file for writing,
 *      otherwise a new file will be created.
 *
 * @param file_name the protected file name
 * @param meta_file_name the metadata file name
 * @param key_policy seal key policy, if SGX_KEYPOLICY_MRSIGNER is specified, will call sgx_fopen_auto_key() to
 *      create or open the file.
 * @return the file handler
 *
 * @throw throw FileSystemException if failed
 */
static SGX_FILE* OpenFileForAppend(const char* file_name, const char* meta_file_name, uint16_t key_policy) {
    // Get enclave self report
    const sgx_report_t* report = sgx_self_report();
    if (!report) {
        throw FileSystemException("Failed to get self Enclave Report");
    }

    // If the metadata file exists, check the metadata items in file
    std::optional<FileMetaData> metafile;
    if (Exists(Path(meta_file_name))) {
        metafile = FileMetaData::FromFile(meta_file_name);
        if (!metafile.has_value()) {
            throw FileSystemException("The file exists, but failed to read its metadata file");
        }
        sgx_key_request_t key_request = metafile.value().GetKeyRequest();
        if (key_request.key_policy != key_policy) {
            throw FileSystemException("Invalid parameter, key_policy is not consistent with the existing file");
        }
        if (key_request.key_policy == SGX_KEYPOLICY_MRSIGNER &&
            metafile.value().GetLegacyMode() != 1) {
            throw FileSystemException("Invalid metadata file, key_policy and legacy mode are not consistent");
        }
        if (key_request.isv_svn != report->body.isv_svn ||
            memcmp(&key_request.cpu_svn, &report->body.cpu_svn, sizeof(sgx_isv_svn_t) ) != 0) {
            throw FileSystemException("A file with the same name already exists and was created by another enclave.");
        }
    }

    // Currently, set legacy_mode = 1 when key_policy is SGX_KEYPOLICY_MRSIGNER.
    // In this case, call API sgx_fopen_auto_key() to create or open the file, so that the file
    // can be opened by other applications which use the same API.
    FileMetaData metadata;
    SGX_FILE* file = nullptr;
    if (key_policy == SGX_KEYPOLICY_MRSIGNER) {
        if (!metafile.has_value()) {
            // key_id will be ignored in this case, so use 0 as the key_id.
            sgx_key_id_t key_id = {0};
            metadata = FileMetaData(1, key_policy, key_id, report->body.isv_svn, report->body.cpu_svn);
        }

        // Call sgx_fopen_auto_key() to create or open the file.
        // In this API, a key will be derived with SGX_KEYPOLICY_MRSIGNER
        file = sgx_fopen_auto_key(file_name, "a");
    }
    else {
        sgx_status_t status = SGX_SUCCESS;
        sgx_key_request_t key_request = {0};
        if (!metafile.has_value()) {
            // Use a random number for key_id
            sgx_key_id_t key_id = {0};
            status = sgx_read_rand(key_id.id, sizeof(key_id));
            if (status != SGX_SUCCESS) {
                throw FileSystemException("Failed to read random number generator");
            }
            metadata = FileMetaData(0, key_policy, key_id, report->body.isv_svn, report->body.cpu_svn);
            key_request = metadata.GetKeyRequest();
        }
        else {
            key_request = metafile.value().GetKeyRequest();
        }

        // Get a seal key
        sgx_key_128bit_t seal_key = {0};
        status = sgx_get_key(&key_request, &seal_key);
        if (status != SGX_SUCCESS) {
            throw FileSystemException("Failed to get seal key");
        }

        // Call sgx_fopen() to create or open the file
        file = sgx_fopen(file_name, "a", &seal_key);

        // Reset seal_key with 0
        memset_s(seal_key, sizeof(sgx_key_128bit_t), 0, sizeof(sgx_key_128bit_t));
    }
    if (!file) {
        throw FileSystemException("Failed to open the file for writing");
    }

    // For new metadata, save it to the metadata file
    if (!metafile.has_value()) {
        if (!metadata.ToFile(meta_file_name)) {
            sgx_fclose(file);
            Remove(Path(file_name));
            throw FileSystemException("Failed to create metadata file");
        }
    }

    return file;
}
}

ProtectedFileWriter::ProtectedFileWriter(const char* file_name, FileMode file_mode, uint16_t key_policy)
    : file_(nullptr), file_name_(file_name) {
    sgx_status_t status = SGX_SUCCESS;

    if (!file_name) {
        throw FileSystemException("File name cannot be null");
    }

    // check key_policy is valid or not
    if (!is_valid_key_policy(key_policy)) {
        throw FileSystemException("Invalid parameter, key_policy is invalid");
    }

    // Metadata file name
    Path metadata_file{file_name};
    metadata_file += FS_METADATA_FILE_EXT;

    // Open the file for writing
    if (file_mode == FileMode::CreateNew) {
        // Throw an exception if the file exists in CreateNew mode
        if (Exists(Path(file_name)) || Exists(metadata_file)) {
            throw FileSystemException("The file already exists");
        }
        file_ = detail::OpenFileForWrite(file_name, metadata_file.c_str(), key_policy);
    }
    else if (file_mode == FileMode::OpenOrCreate) {
        file_ = detail::OpenFileForWrite(file_name, metadata_file.c_str(), key_policy);
    }
    else if (file_mode == FileMode::Append) {
        file_ = detail::OpenFileForAppend(file_name, metadata_file.c_str(), key_policy);
    }
    else {
        throw FileSystemException("Invalid file mode");
    }
}

ProtectedFileWriter::~ProtectedFileWriter() {
    if (file_) {
        sgx_fclose(file_);
    }
}

void ProtectedFileWriter::Write(const void* data, size_t size) const {
    if (!file_ || !data || size == 0) {
        throw FileSystemException("Invalid write parameters");
    }
    if (sgx_fwrite(data, 1, size, file_) != size) {
        throw FileSystemException("Write error");
    }
}

void ProtectedFileWriter::Flush() const {
    if (!file_) {
        throw FileSystemException("File not open");
    }
    if (sgx_fflush(file_) != 0) {
        throw FileSystemException("Flush error");
    }
}

void ProtectedFileWriter::Close() {
    if (file_) {
        if (sgx_fclose(file_) != 0) {
            throw FileSystemException("Close error");
        }
        file_ = nullptr;
    }
}

} // namespace filesystem_t
} // namespace ssgx