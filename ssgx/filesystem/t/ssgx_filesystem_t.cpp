#include "ssgx_filesystem_t.h"

#include "sgx_lfence.h"
#include "sgx_tprotected_fs.h"
#include "sgx_trts.h"
#include "sgx_utils.h"

#include "ssgx_filesystem_t_t.h"
#include "ssgx_utils_t.h"

#include "../../common/internal_check.h"
#include "FileMetaData.h"
#include "filesystem_constant.h"

namespace ssgx {
namespace filesystem_t {

bool CreateDirectory(const Path& p) {
    if (p.Empty()) {
        throw FileSystemException("The input file path is empty");
    }

    FileStatus file_status = Status(p);
    if (Exists(file_status)) {
        if (IsDirectory(file_status)) {
            return false;
        } else {
            throw FileSystemException("This path already exists and is not a directory type");
        }
    }

    int ret;
    sgx_status_t sgx_status = ssgx_ocall_create_directory(&ret, p.c_str());
    if (sgx_status != SGX_SUCCESS) {
        throw FileSystemException(ssgx::utils_t::FormatStr(
            "Enclave ssgx_ocall_create_directory function call failed, sgx status: 0x%x", sgx_status));
    }
    if (ret < 0) {
        throw FileSystemException(
            ssgx::utils_t::FormatStr("Enclave ssgx_ocall_create_directory function call failed, error code: %d", ret));
    }

    return true;
}

bool Exists(const FileStatus& s) noexcept {
    return s.Type() != FileType::None && s.Type() != FileType::NotFound;
}

bool Exists(const Path& p) {
    return Exists(Status(p));
}

bool Remove(const Path& p) {
    if (p.Empty()) {
        throw FileSystemException("The input file path is empty");
    }
    if (!Exists(p)) {
        return false;
    }

    int ret;
    sgx_status_t sgx_status = ssgx_ocall_remove_file(&ret, p.c_str());
    if (sgx_status != SGX_SUCCESS) {
        throw FileSystemException(ssgx::utils_t::FormatStr(
            "Enclave ssgx_ocall_remove_file function call failed, sgx status: 0x%x", sgx_status));
    }
    if (ret < 0) {
        throw FileSystemException(
            ssgx::utils_t::FormatStr("Enclave ssgx_ocall_remove_file function call failed, error code: %d", ret));
    }

    return true;
}

bool RemoveProtectedFile(const Path& file_name) {
    Path metadata_file(file_name);
    metadata_file += FS_METADATA_FILE_EXT;
    return Remove(file_name) && Remove(metadata_file);
}

uintmax_t FileSize(const Path& p) {
    if (p.Empty()) {
        throw FileSystemException("The input file path is empty");
    }

    int ret;
    long int file_size;
    sgx_status_t sgx_status = ssgx_ocall_get_file_size(&ret, p.c_str(), &file_size);
    if (sgx_status != SGX_SUCCESS) {
        throw FileSystemException(ssgx::utils_t::FormatStr(
            "Enclave ssgx_ocall_get_file_size function call failed, sgx status: 0x%x", sgx_status));
    }
    if (ret < 0) {
        throw FileSystemException(
            ssgx::utils_t::FormatStr("Enclave ssgx_ocall_get_file_size function call failed, error code: %d", ret));
    }

    return file_size;
}

bool IsDirectory(const FileStatus& s) noexcept {
    return s.Type() == FileType::Directory;
}

bool IsDirectory(const Path& p) {
    return IsDirectory(Status(p));
}

bool IsEmpty(const Path& p) {
    if (p.Empty()) {
        throw FileSystemException("The input file path is empty");
    }

    int ret;
    uint32_t is_empty;
    sgx_status_t sgx_status = ssgx_ocall_is_directory_or_regular_file_empty(&ret, p.c_str(), &is_empty);
    if (sgx_status != SGX_SUCCESS) {
        throw FileSystemException(ssgx::utils_t::FormatStr(
            "Enclave ssgx_ocall_is_directory_or_regular_file_empty function call failed, sgx status: 0x%x",
            sgx_status));
    }
    if (ret < 0) {
        throw FileSystemException(ssgx::utils_t::FormatStr(
            "Enclave ssgx_ocall_is_directory_or_regular_file_empty function call failed, error code: %d", ret));
    }

    return is_empty == 1;
}

bool IsRegularFile(const FileStatus& s) noexcept {
    return s.Type() == FileType::Regular;
}

bool IsRegularFile(const Path& p) {
    return IsRegularFile(Status(p));
}

FileStatus Status(const Path& p) {
    int ret = 0;
    uint32_t file_type = 0;
    uint32_t file_perm = 0;

    if (p.Empty()) {
        throw FileSystemException("The input file path is empty");
    }

    sgx_status_t sgx_status = ssgx_ocall_get_file_status(&ret, p.c_str(), &file_type, &file_perm);
    if (sgx_status != SGX_SUCCESS) {
        throw FileSystemException(ssgx::utils_t::FormatStr(
            "Enclave ssgx_ocall_get_file_status function call failed, sgx status: 0x%x", sgx_status));
    }
    if (ret < 0) {
        throw FileSystemException(
            ssgx::utils_t::FormatStr("Enclave ssgx_ocall_get_file_status function call failed, error code: %d", ret));
    }

    if (file_type <= static_cast<uint32_t>(FileType::None) || file_type >= static_cast<uint32_t>(FileType::Unknown)) {
        throw FileSystemException(ssgx::utils_t::FormatStr("The file type is invalid, file type: %d", file_type));
    }

    if ((file_perm & ~static_cast<uint32_t>(Perms::Mask)) != 0) {
        throw FileSystemException(
            ssgx::utils_t::FormatStr("The file permissions are invalid, file permissions: 0%o", file_perm));
    }

    return FileStatus(static_cast<FileType>(file_type), static_cast<Perms>(file_perm));
}

FileStatus SymlinkStatus(const Path& p) {
    int ret = 0;
    uint32_t file_type = 0;
    uint32_t file_perm = 0;

    if (p.Empty()) {
        throw FileSystemException("The input file path is empty");
    }

    sgx_status_t sgx_status = ssgx_ocall_get_symlink_file_status(&ret, p.c_str(), &file_type, &file_perm);
    if (sgx_status != SGX_SUCCESS) {
        throw FileSystemException(ssgx::utils_t::FormatStr(
            "Enclave ssgx_ocall_get_file_status function call failed, sgx status: 0x%x", sgx_status));
    }
    if (ret < 0) {
        throw FileSystemException(
            ssgx::utils_t::FormatStr("Enclave ssgx_ocall_get_file_status function call failed, error code: %d", ret));
    }

    if (file_type <= static_cast<uint32_t>(FileType::None) || file_type >= static_cast<uint32_t>(FileType::Unknown)) {
        throw FileSystemException(ssgx::utils_t::FormatStr("The file type is invalid, file type: %d", file_type));
    }

    if ((file_perm & ~static_cast<uint32_t>(Perms::Mask)) != 0) {
        throw FileSystemException(
            ssgx::utils_t::FormatStr("The file permissions are invalid, file permissions: 0%o", file_perm));
    }

    return FileStatus(static_cast<FileType>(file_type), static_cast<Perms>(file_perm));
}

} // namespace filesystem_t
} // namespace ssgx