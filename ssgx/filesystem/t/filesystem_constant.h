#include <cstdint>

#ifndef SSGX_FS_CONSTANTS_H
#define SSGX_FS_CONSTANTS_H

namespace ssgx {
namespace filesystem_t {

/* ssgx protected file metadata tag */
static constexpr uint8_t FS_META_TAG[8] = {0x7b, 0x1d, 0x3f, 0x75, 0xe3, 0x2e, 0x29, 0x20};

/* Current protected file metadata version */
static constexpr uint8_t FS_METADATA_VERSION = 0x01;

// Max file size is 100 KB
static constexpr std::size_t FS_MAX_FILE_SIZE = 100 * 1024;

// Tha additional MAC data for seal/unseal
static constexpr const char* FS_SEAL_ADD_MAC_DATA = "Safeheron ssgx filesystem sealed data";

// the protected file's metadata file extension
static constexpr const char* FS_METADATA_FILE_EXT = ".pfsmeta";

} // namespace filesystem_t
} // namespace ssgx

#endif  // SSGX_FS_CONSTANTS_H
