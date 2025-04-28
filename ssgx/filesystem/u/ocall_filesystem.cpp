#include <string>

#include "ssgx_filesystem_t_enum.h"
#include "ssgx_filesystem_t_u.h"

#include "fs_auxiliary.h"
#ifdef __unix__
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <unistd.h>
#else
#error "We currently only support Linux."
#endif

using Stat = struct stat;
using ssgx::filesystem_t::FileType;
using ssgx::filesystem_t::Perms;

// Max buffer size for once reading/writing
#define SSGX_FS_BUFFER_SIZE (1024 * 4)

// Max file size is 100 KB
#define SSGX_FS_MAX_FILE_SIZE (100 * 1024)

extern "C" int ssgx_ocall_get_file_status(const char* path, uint32_t* file_type, uint32_t* file_permission) {
    int ret = 0;
    Stat s_buf = {0};

    if (!path || strnlen(path, 1) == 0 || !file_type || !file_permission) {
        return -1;
    }

    ret = stat(path, &s_buf);
    if (ret != 0) {
        // ENOENT: A component of pathname does not exist or is a dangling symbolic link.
        // ENOTDIR: A component of the path prefix of pathname is not a directory.
        // Link: https://man7.org/linux/man-pages/man2/stat.2.html
        if (errno == ENOENT || errno == ENOTDIR) {
            *file_type = static_cast<uint32_t>(FileType::NotFound);
            *file_permission = static_cast<uint32_t>(Perms::None);
            return 0;
        } else {
            return -2;
        }
    }

    switch (s_buf.st_mode & S_IFMT) {
    case S_IFREG:
        *file_type = static_cast<uint32_t>(FileType::Regular);
        break;
    case S_IFDIR:
        *file_type = static_cast<uint32_t>(FileType::Directory);
        break;
    case S_IFBLK:
        *file_type = static_cast<uint32_t>(FileType::Block);
        break;
    case S_IFCHR:
        *file_type = static_cast<uint32_t>(FileType::Character);
        break;
    case S_IFIFO:
        *file_type = static_cast<uint32_t>(FileType::Fifo);
        break;
    case S_IFSOCK:
        *file_type = static_cast<uint32_t>(FileType::Socket);
        break;
    case S_IFLNK:
        *file_type = static_cast<uint32_t>(FileType::Symlink);
        break;
    default:
        *file_type = static_cast<uint32_t>(FileType::Unknown);
        break;
    }

    *file_permission = s_buf.st_mode & static_cast<uint32_t>(Perms::Mask);
    return 0;
}

extern "C" int ssgx_ocall_get_symlink_file_status(const char* path, uint32_t* file_type, uint32_t* file_permission) {
    int ret = 0;
    Stat s_buf = {0};

    if (!path || strnlen(path, 1) == 0 || !file_type || !file_permission) {
        return -1;
    }

    ret = lstat(path, &s_buf);
    if (ret != 0) {
        // ENOENT: A component of pathname does not exist or is a dangling symbolic link.
        // ENOTDIR: A component of the path prefix of pathname is not a directory.
        // Link: https://man7.org/linux/man-pages/man2/stat.2.html
        if (errno == ENOENT || errno == ENOTDIR) {
            *file_type = static_cast<uint32_t>(FileType::NotFound);
            *file_permission = static_cast<uint32_t>(Perms::None);
            return 0;
        } else {
            return -2;
        }
    }

    switch (s_buf.st_mode & S_IFMT) {
    case S_IFREG:
        *file_type = static_cast<uint32_t>(FileType::Regular);
        break;
    case S_IFDIR:
        *file_type = static_cast<uint32_t>(FileType::Directory);
        break;
    case S_IFBLK:
        *file_type = static_cast<uint32_t>(FileType::Block);
        break;
    case S_IFCHR:
        *file_type = static_cast<uint32_t>(FileType::Character);
        break;
    case S_IFIFO:
        *file_type = static_cast<uint32_t>(FileType::Fifo);
        break;
    case S_IFSOCK:
        *file_type = static_cast<uint32_t>(FileType::Socket);
        break;
    case S_IFLNK:
        *file_type = static_cast<uint32_t>(FileType::Symlink);
        break;
    default:
        *file_type = static_cast<uint32_t>(FileType::Unknown);
        break;
    }

    *file_permission = s_buf.st_mode & static_cast<uint32_t>(Perms::Mask);
    return 0;
}

/*
 *  is the directory or regular file empty
 */
extern "C" int ssgx_ocall_is_directory_or_regular_file_empty(const char* path, uint32_t* is_empty) {
    int ret = 0;
    Stat s_buf = {0};

    if (!path || strnlen(path, 1) == 0 || !is_empty) {
        return -1;
    }

    ret = stat(path, &s_buf);
    if (ret != 0) {
        return -2;
    }
    if (S_ISDIR(s_buf.st_mode)) {
        return ssgx::filesystem_u::is_directory_empty(path, is_empty) ? 0 : -3;
    } else if (S_ISREG(s_buf.st_mode)) {
        *is_empty = (static_cast<uintmax_t>(s_buf.st_size) == 0 ? 1 : 0);
    } else {
        return -4;
    }

    return 0;
}

/*
 *  create a directory
 */
extern "C" int ssgx_ocall_create_directory(const char* path) {
    int ret = 0;

    if (!path || strnlen(path, 1) == 0) {
        return -1;
    }

    ret = mkdir(path, static_cast<int>(Perms::All));
    if (ret != 0) {
        return -2;
    }

    return 0;
}

extern "C" int ssgx_ocall_get_file_size(const char* path, long int* file_size) {
    int ret = 0;
    Stat s_buf = {0};

    if (!path || strnlen(path, 1) == 0 || !file_size) {
        return -1;
    }

    ret = stat(path, &s_buf);
    if (ret != 0) {
        return -2;
    }
    if (!S_ISREG(s_buf.st_mode)) {
        return -3;
    }
    *file_size = s_buf.st_size;
    return 0;
}

/*
 *  delete a file
 */
extern "C" int ssgx_ocall_remove_file(const char* path) {
    int ret = 0;

    if (!path || strnlen(path, 1) == 0) {
        return -1;
    }

    ret = remove(path);
    if (ret != 0) {
        return -2;
    }

    return 0;
}

/*
 *  read a normal file
 */
extern "C" int ssgx_ocall_read_file(const char* path, int is_binary, uint8_t** data, size_t* size) {
    *size = 0;
    *data = nullptr;

    if (!path || strnlen(path, 1) == 0 || !data || !size) {
        return -1;
    }

    // return error if failed to open the file
    FILE* fp = (is_binary == 1) ? fopen(path, "rb") : fopen(path, "r");
    if (!fp) {
        return -2;
    }

    // lock this file with a share locker,
    // this file can only be read by other processes.
    if (flock(fileno(fp), LOCK_SH|LOCK_NB) == -1) {
        fclose(fp);
        return -3;
    }

    // get file status
    Stat s_buf;
    if (fstat(fileno(fp), &s_buf) != 0) {
        flock(fileno(fp), LOCK_UN);
        fclose(fp);
        return -4;
    }

    // check file size
    // return success if this file is empty
    // return error if this file size exceed SSGX_FS_MAX_FILE_SIZE
    const size_t file_size = s_buf.st_size;
    if (file_size == 0) {
        flock(fileno(fp), LOCK_UN);
        fclose(fp);
        return 0;
    } else if (file_size > SSGX_FS_MAX_FILE_SIZE) {
        flock(fileno(fp), LOCK_UN);
        fclose(fp);
        return -5;
    }

    // malloc buffer for file data
    auto* file_buf = static_cast<uint8_t*>(malloc(file_size));
    if (!file_buf) {
        flock(fileno(fp), LOCK_UN);
        fclose(fp);
        return -6;
    }
    memset(file_buf, 0, file_size);

    // read data from file
    size_t total_read = 0;
    while (total_read < file_size) {
        const size_t chunk = std::min(file_size - total_read, static_cast<size_t>(SSGX_FS_BUFFER_SIZE));
        const size_t count = fread(file_buf + total_read, 1, chunk, fp);
        if (count != chunk) {
            flock(fileno(fp), LOCK_UN);
            free(file_buf);
            fclose(fp);
            return -7;
        }
        total_read += count;
    }

    // validate file size
    if (total_read != file_size) {
        flock(fileno(fp), LOCK_UN);
        free(file_buf);
        fclose(fp);
        return -8;
    }

    // success
    *data = file_buf;
    *size = file_size;
    flock(fileno(fp), LOCK_UN);
    fclose(fp);

    return 0;
}

/*
 *  write data to a normal file
 */
extern "C" int ssgx_ocall_write_file(const char* path, int is_binary, const uint8_t* data, size_t size) {
    if (!path || strnlen(path, 1) == 0) {
        return -1;
    }
    if (size > 0 && !data) {
        return -2;
    }
    if (size > SSGX_FS_MAX_FILE_SIZE) {
        return -3;
    }

    // open file for writing, return error if failed
    FILE* fp = (is_binary == 1) ? fopen(path, "wb") : fopen(path, "w");
    if (!fp) {
        return -4;
    }

    // lock this file with an exclusive locker,
    // this file cannot be read or written by other processes.
    if (flock(fileno(fp), LOCK_EX|LOCK_NB) == -1) {
        fclose(fp);
        return -5;
    }

    // write data to file
    // remove this file if encounter errors
    size_t write_size = 0;
    while (write_size < size) {
        const size_t chunk_size = std::min(size - write_size, static_cast<size_t>(SSGX_FS_BUFFER_SIZE));
        const size_t count = fwrite(data + write_size, 1, chunk_size, fp);
        if (count != chunk_size) {
            flock(fileno(fp), LOCK_UN);
            fclose(fp);
            remove(path);
            return -6;
        }
        write_size += count;
    }

    // success
    flock(fileno(fp), LOCK_UN);
    fclose(fp);

    return 0;
}
