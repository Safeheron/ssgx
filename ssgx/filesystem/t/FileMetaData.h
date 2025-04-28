#ifndef SSGXLIB_SSGX_FILESYSTEM_T_FILE_META_DATA_T_H
#define SSGXLIB_SSGX_FILESYSTEM_T_FILE_META_DATA_T_H

#include <optional>
#include <stdint.h>
#include <string>

#include "sgx_key.h"

namespace ssgx {
namespace filesystem_t {

class FileMetaData {
  public:
    FileMetaData() = default;
    FileMetaData(FileMetaData&&) noexcept = default;
    FileMetaData& operator=(FileMetaData&&) noexcept = default;
    FileMetaData& operator=(const FileMetaData&) noexcept = default;

    explicit FileMetaData(uint8_t legacy_mode, uint16_t policy, const sgx_key_id_t& key_id, const sgx_isv_svn_t& isv_svn,
            const sgx_cpu_svn_t& cpu_svn);

    sgx_key_request_t GetKeyRequest() const;
    uint8_t GetLegacyMode() const;
    uint8_t GetVersion() const;
    bool ToFile(const std::string& path_name);
    static std::optional<FileMetaData> FromFile(const std::string& path_name);

  private:
    uint8_t version_ = 0;
    uint8_t legacy_mode_ = 0;
    uint16_t key_policy_ = 0;
    sgx_key_id_t key_id_{0};
    sgx_isv_svn_t isv_svn_ = 0;
    sgx_cpu_svn_t cpu_svn_{0};
};

} // namespace filesystem_t
} // namespace ssgx

#endif // SSGXLIB_SSGX_FILESYSTEM_T_FILE_META_DATA_T_H
