#include "fs_auxiliary.h"
#ifdef __unix__
#include <cstring>
#include <dirent.h>
#include <unistd.h>
#else
#error "We currently only support Linux."
#endif

namespace ssgx {
namespace filesystem_u {

bool is_directory_empty(const char* path, uint32_t* is_empty) {
    DIR* dp = nullptr;
    dp = opendir(path);
    if (!dp) {
        return false;
    }

    dirent* fp_dir;
    *is_empty = 1;
    while ((fp_dir = readdir(dp)) != nullptr) {
        if (strcmp(".", fp_dir->d_name) != 0 && strcmp("..", fp_dir->d_name) != 0) {
            *is_empty = 0;
            break;
        }
    }
    closedir(dp);
    return true;
}

} // namespace filesystem_u
} // namespace ssgx