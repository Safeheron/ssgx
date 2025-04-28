#include <vector>

#include "ssgx_filesystem_t.h"

#include "../../common/tseal_migration_attr.h"
#include "crypto-suites/common/MemoryWalker.h"
#include "crypto-suites/common/MemoryWriter.h"
#include "crypto-suites/crypto-hash/sha256.h"
#include "FileMetaData.h"
#include "filesystem_constant.h"

namespace ssgx {
namespace filesystem_t {

FileMetaData::FileMetaData(uint8_t legacy_mode, uint16_t policy, const sgx_key_id_t& key_id, const sgx_isv_svn_t& isv_svn,
                           const sgx_cpu_svn_t& cpu_svn) : version_(FS_METADATA_VERSION) {
    legacy_mode_ = legacy_mode;
    key_policy_ = policy;
    key_id_ = key_id;
    isv_svn_ = isv_svn;
    cpu_svn_ = cpu_svn;
}

sgx_key_request_t FileMetaData::GetKeyRequest() const {
    sgx_key_request_t key_request = {0};

    key_request.key_name = SGX_KEYSELECT_SEAL;
    key_request.key_policy = key_policy_;
    memcpy(&key_request.key_id, &key_id_, sizeof(sgx_key_id_t));
    memcpy(&key_request.cpu_svn, &cpu_svn_, sizeof(sgx_cpu_svn_t));
    memcpy(&key_request.isv_svn, &isv_svn_, sizeof(sgx_isv_svn_t));

    // Bitmask indicating which attributes the seal key should be bound to. The recommendation is to set all the
    // attribute flags, except Mode 64 bit, Provision Key and Launch key, and none of the XFRM attributes.
    // See documentation https://download.01.org/intel-sgx/latest/linux-latest/docs/Intel_SGX_Developer_Guide.pdf P30.
    //
    // Refer to: https://github.com/intel/linux-sgx/blob/main/sdk/protected_fs/sgx_tprotected_fs/file_crypto.cpp#L214
    key_request.attribute_mask.flags = TSEAL_DEFAULT_FLAGSMASK;
    key_request.attribute_mask.xfrm = 0x0;
    key_request.misc_mask = TSEAL_DEFAULT_MISCMASK;

    return key_request;
}

uint8_t FileMetaData::GetLegacyMode() const {
    return legacy_mode_;
}

uint8_t FileMetaData::GetVersion() const {
    return version_;
}

bool FileMetaData::ToFile(const std::string& path_name) {
    bool ok = true;
    if (path_name.empty())
        return false;

    // calc metadata size
    uint32_t output_len = 0;
    // sizeof(META_TAG) bytes for tag
    output_len += sizeof(FS_META_TAG);
    // sizeof(uint8_t) for version
    output_len += sizeof(uint8_t);
    // sizeof(uint8_t) for legacy_mode
    output_len += sizeof(uint8_t);
    // sizeof(uint16_t) for key_policy
    output_len += sizeof(uint16_t);
    // sizeof(sgx_key_id_t) for key_id
    output_len += sizeof(sgx_key_id_t);
    // sizeof(sgx_isv_svn_t) for isv_svn
    output_len += sizeof(sgx_isv_svn_t);
    // sizeof(sgx_cpu_svn_t) for cpu_svn
    output_len += sizeof(sgx_cpu_svn_t);
    // sha256 size for whole data
    output_len += 32;

    std::unique_ptr<uint8_t[]> p_output(new uint8_t[output_len]);
    safeheron::memory::MemoryWriter mem_writer(p_output.get(), output_len);

    // tag
    ok = mem_writer.write_buf(FS_META_TAG, sizeof(FS_META_TAG));
    if (!ok)
        return false;

    // version
    ok = mem_writer.write_byte(version_);
    if (!ok)
        return false;

    // legacy_mode
    ok = mem_writer.write_byte(legacy_mode_);
    if (!ok)
        return false;

    // key policy
    ok = mem_writer.write_buf(reinterpret_cast<const uint8_t*>(&key_policy_), sizeof(uint16_t));
    if (!ok)
        return false;

    // key id
    ok = mem_writer.write_buf(reinterpret_cast<const uint8_t*>(&key_id_), sizeof(sgx_key_id_t));
    if (!ok)
        return false;

    // isv_svn
    ok = mem_writer.write_buf(reinterpret_cast<const uint8_t*>(&isv_svn_), sizeof(sgx_isv_svn_t));
    if (!ok)
        return false;

    // cpu_svn
    ok = mem_writer.write_buf(reinterpret_cast<const uint8_t*>(&cpu_svn_), sizeof(sgx_cpu_svn_t));
    if (!ok)
        return false;

    // calculate SHA256 for the whole metadata and write the result to the end
    uint8_t md[32] = {0};
    safeheron::hash::CSHA256 sha256;
    sha256.Write(p_output.get(), output_len - 32);
    sha256.Finalize(md);
    ok = mem_writer.write_buf(md, sizeof(md));
    if (!ok)
        return false;

    // write metadata to file
    std::vector<uint8_t> file_content(p_output.get(), p_output.get() + output_len);
    PlainFileWriter writer(path_name);
    writer.WriteAllBytes(file_content);

    return true;
}

std::optional<FileMetaData> FileMetaData::FromFile(const std::string& path_name) {
    bool ok = true;
    if (path_name.empty())
        return std::nullopt;

    // read metadata from file
    std::vector<uint8_t> file_content;
    try {
        PlainFileReader reader(path_name);
        file_content = reader.ReadAllBytes();
    } catch (FileSystemException e) {
        return std::nullopt;
    }

    // parse metadata to items
    const uint8_t* p_out = nullptr;
    safeheron::memory::MemoryWalker walker(file_content.data(), file_content.size());

    // tag
    ok = walker.move_buf(p_out, sizeof(FS_META_TAG));
    if (!ok || !p_out || (memcmp(p_out, FS_META_TAG, sizeof(FS_META_TAG)) != 0))
        return std::nullopt;

    // version
    uint8_t version;
    ok = walker.move_byte(version);
    if (!ok || version != FS_METADATA_VERSION)
        return std::nullopt;

    // legacy_mode
    uint8_t legacy_mode = 0;
    ok = walker.move_byte(legacy_mode);
    if (!ok)
        return std::nullopt;

    // key policy
    uint16_t key_policy = 0;
    ok = walker.move_buf(p_out, sizeof(uint16_t));
    if (!ok || !p_out)
        return std::nullopt;
    memcpy(&key_policy, p_out, sizeof(uint16_t));

    // key id
    sgx_key_id_t key_id = {0};
    ok = walker.move_buf(p_out, sizeof(sgx_key_id_t));
    if (!ok || !p_out)
        return std::nullopt;
    memcpy(&key_id, p_out, sizeof(sgx_key_id_t));

    // isv_svn
    sgx_isv_svn_t isv_svn = {0};
    ok = walker.move_buf(p_out, sizeof(sgx_isv_svn_t));
    if (!ok || !p_out)
        return std::nullopt;
    memcpy(&isv_svn, p_out, sizeof(sgx_isv_svn_t));

    // cpu_svn
    sgx_cpu_svn_t cpu_svn = {0};
    ok = walker.move_buf(p_out, sizeof(sgx_cpu_svn_t));
    if (!ok || !p_out)
        return std::nullopt;
    memcpy(&cpu_svn, p_out, sizeof(sgx_cpu_svn_t));

    // SHA256 for whole metadata
    ok = walker.move_buf(p_out, 32);
    if (!ok || !p_out)
        return std::nullopt;

    // Validate SHA256 for whole metadata
    uint8_t md[32] = {0};
    safeheron::hash::CSHA256 sha256;
    sha256.Write(file_content.data(), file_content.size() - 32);
    sha256.Finalize(md);
    if (memcmp(md, p_out, 32) != 0) {
        return std::nullopt;
    }

    return FileMetaData(legacy_mode, key_policy, key_id, isv_svn, cpu_svn);
}

} // namespace filesystem_t
} // namespace ssgx
