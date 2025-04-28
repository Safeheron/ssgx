#ifndef SSGXLIB_SSGX_TFILESYSTEM_ENUM_H
#define SSGXLIB_SSGX_TFILESYSTEM_ENUM_H

namespace ssgx {
namespace filesystem_t {

/**
 * @brief File type enum
 */
enum class FileType {
    None = 0,  ///< default value
    NotFound, ///< the file is not exist
    Regular,   ///< the file is a regular file
    Directory, ///< the file is a directory file
    Symlink,   ///< the file is a symlink file
    Block,     ///< the file is a block file
    Character, ///< the file is a character file
    Fifo,      ///< the file is a fifo file
    Socket,    ///< the file is a socket file
    Unknown,   ///< the type of file is unknown
};

/**
 * @brief File permissions (Octal)
 */
enum class Perms : unsigned {
    None = 0, ///< No permissions

    OwnerRead = 0400,  ///< The file owner has read permission
    OwnerWrite = 0200, ///< The file owner has write permission
    OwnerExec = 0100,  ///< The file owner has execute permission
    OwnerAll = 0700,   ///< The file owner has read, write and execute permission

    GroupRead = 040,  ///< The file owner's user group has read permissions
    GroupWrite = 020, ///< The file owner's user group has write permissions
    GroupExec = 010,  ///< The file owner's user group has execute permissions
    GroupAll = 070,   ///< The file owner's user group has read, write and execute
                       ///< permission

    OthersRead = 04,  ///< Others have read permission
    OthersWrite = 02, ///< Others have write permission
    OthersExec = 01,  ///< Others have execute permission
    OthersAll = 07,   ///< Others have read, write and execute permission

    All = 0777, ///< All users have read, write and execute permission

    SetUid = 04000,    ///< When this bit is set, the file has the file owner's
                        ///< permissions during execution
    SetGid = 02000,    ///< When this bit is set, files created in this directory
                        ///< belong to the same group as the directory.
    StickyBit = 01000, ///< When this bit is set, only the owner of the file in
                        ///< the directory or root can delete or move the file.
    Mask = 07777,       ///< The file permissions represent the bits
    Unknown = 0xFFFF,   ///< Invalid permission
};

/**
 * @brief The mode for file writing operation
 */
enum class FileMode {
    CreateNew = 1,          ///< Specifies that the operating system should create a new file.  If the file already exists, an FileSystemException exception is thrown.
    OpenOrCreate = 2,       ///< Specifies that the operating system should open a file if it exists; otherwise, a new file will be created.
    Append = 3              ///< Opens the file if it exists and seeks to the end of the file, or creates a new file.
};

} // namespace filesystem_t
} // namespace ssgx
#endif // SSGXLIB_SSGX_TFILESYSTEM_ENUM_H
