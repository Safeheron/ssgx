#ifndef SSGXLIB_FS_AUXILIARY_H
#define SSGXLIB_FS_AUXILIARY_H

#include <cstdint>

namespace ssgx {
namespace filesystem_u {

bool is_directory_empty(const char* path, uint32_t* is_empty);

}
} // namespace ssgx

#endif // SSGXLIB_FS_AUXILIARY_H
