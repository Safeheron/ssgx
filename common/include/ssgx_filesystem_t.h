
#ifndef SAFEHERON_SGX_TRUSTED_FILESTREAM_H
#define SAFEHERON_SGX_TRUSTED_FILESTREAM_H
#include <string>

#include "sgx_key.h"
#include "sgx_tprotected_fs.h"

#include "ssgx_filesystem_t_enum.h"
namespace ssgx {
/**
 * @brief This module is designed to operate directly on files and directories in enclave.
 */
namespace filesystem_t {
/**
 * @brief A class to store file type and permissions
 */
class FileStatus {
  public:
    /**
     * @brief Construct a FileStatus and initialize it
     */
    FileStatus() noexcept : FileStatus(FileType::None) {
    }

    /**
     * @brief Construct a FileStatus and initialize it
     */
    explicit FileStatus(FileType ft, Perms prms = Perms::Unknown) noexcept : ft_(ft), prms_(prms) {
    }

    /**
     * @brief A copy constructor
     */
    FileStatus(const FileStatus&) noexcept = default;

    /**
     * @brief A move constructor
     */
    FileStatus(FileStatus&&) noexcept = default;

    /**
     * @brief Destruction
     */
    ~FileStatus() {
    }

    /**
     * @brief A copy assignment operator
     * @return A FileStatus object copied from another FileStatus object
     */
    FileStatus& operator=(const FileStatus&) noexcept = default;

    /**
     * @brief A move assignment operator
     * @return A FileStatus object moved from another FileStatus object
     */
    FileStatus& operator=(FileStatus&&) noexcept = default;

    /**
     * @brief Return the file type of file
     * @return File type
     */
    FileType Type() const noexcept {
        return ft_;
    }

    /**
     * @brief Return the permissions of file
     * @return Permissions (octal number)
     */
    Perms Permissions() const noexcept {
        return prms_;
    }

    /**
     * @brief Modify the file type in FileStatus
     * @param[in] ft Input file type
     */
    void Type(FileType ft) noexcept {
        ft_ = ft;
    }

    /**
     * @brief Modify the permissions in FileStatus
     * @param[in] p Input permissions (Octal number)
     */
    void Permissions(Perms p) noexcept {
        prms_ = p;
    }

  private:
    FileType ft_; ///< File type
    Perms prms_;  ///< File permissions
};

/**
 * @brief A class for storing path
 */
class Path {
#if defined(_LIBCPP_WIN32API)
#error "We do not support Windows."
    typedef wchar_t value_type;
    static constexpr value_type preferred_separator = L'\\';
#else
    typedef char value_type;
    static constexpr value_type preferred_separator = '/';
#endif
    typedef std::basic_string<value_type> string_type;

  public:
    /**
     * @brief Construct a Path and initialize it
     */
    Path() noexcept {
    }

    /**
     * @brief Construct a Path and initialize it
     */
    Path(const Path& p) : pn_(p.pn_) {
    }

    /**
     * @brief A move constructor
     */
    Path(Path&& p) noexcept : pn_(std::move(p.pn_)) {
    }

    /**
     * @brief A move constructor
     */
    Path(string_type&& source) : pn_(std::move(source)) {
    }

    /**
     * @brief Destruction
     */
    ~Path() = default;

    /**
     * @brief A copy assignment operator
     * @return A Path object copied from another Path object
     */
    Path& operator=(const Path& p) {
        pn_ = p.pn_;
        return *this;
    }

    /**
     * @brief A move assignment operator
     * @return A Path object moved from another Path object
     */
    Path& operator=(Path&& p) noexcept {
        pn_ = std::move(p.pn_);
        return *this;
    }

    /**
     * @brief A move assignment operator
     * @return A Path object moved from another Path object
     */
    Path& operator=(string_type&& source) noexcept {
        pn_ = std::move(source);
        return *this;
    }

    /**
     * @brief A move assignment operator
     * @return A Path object moved from another Path object
     */
    Path& assign(string_type&& source) noexcept {
        pn_ = std::move(source);
        return *this;
    }

    /**
     * @brief Appends operator
     * @param[in] x Path
     * @return The concatenated path
     */
    Path& operator+=(const Path& x) {
        pn_ += x.pn_;
        return *this;
    }

    /**
     * @brief Appends operator
     * @param[in] x Path of string type
     * @return The concatenated path
     */
    Path& operator+=(const string_type& x) {
        pn_ += x;
        return *this;
    }

    /**
     * @brief Appends operator
     * @details
     *      -# If right Path is an absolute path, then left Path = right Path
     *      -# If left Path has filename, then left Path = left Path + '/' + right Path
     *      -# Otherwise left Path = left Path + right Path
     *
     * @param[in] p Path
     * @return A Path object
     */
    Path& operator/=(const Path& p) {
        if (p.IsAbsolute()) {
            pn_ = p.pn_;
            return *this;
        }
        if (HasFilename())
            pn_ += preferred_separator;
        pn_ += p.pn_;
        return *this;
    }

    /**
     * @brief Compare whether the Paths are the same
     * @param[in] lhs Left Path
     * @param[in] rhs Right Path
     * @return Return true if they are the same; otherwise, return false
     */
    friend bool operator==(const Path& lhs, const Path& rhs) noexcept {
        return lhs.String() == rhs.String();
    }

    /**
     * @brief Compare whether the Paths are different
     * @param[in] lhs Left Path
     * @param[in] rhs Right Path
     * @return Return true if they are different; otherwise, return false
     */
    friend bool operator!=(const Path& lhs, const Path& rhs) noexcept {
        return lhs.String() != rhs.String();
    }

    /**
     * @brief Appends operator
     * @details
     *      -# If right Path is an absolute path, then left Path = right Path
     *      -# If left Path has filename, then left Path = left Path + '/' + right Path
     *      -# Otherwise left Path = left Path + right Path
     *
     * @param[in] lhs Left Path
     * @param[in] rhs Right Path
     * @return A Path object
     */
    friend Path operator/(const Path& lhs, const Path& rhs) {
        Path result(lhs);
        result /= rhs;
        return result;
    }

    /**
     * @brief Check if the path is absolute
     * @return Return true if the path is absolute; otherwise, return false
     */
    bool IsAbsolute() const noexcept {
        return !pn_.empty() && pn_.front() == preferred_separator;
    }

    /**
     * @brief Check if the path has a file name
     * @return Return true if it has filename; otherwise, return false
     */
    bool HasFilename() const noexcept {
        return !pn_.empty() && pn_.back() != preferred_separator;
    }

    /**
     * @brief Check if the path is empty
     * @return Return true if it is empty; otherwise return false
     */
    bool Empty() const noexcept {
        return pn_.empty();
    }

    /**
     * @brief Return the string of Path
     * @return The string of Path
     */
    const char* c_str() const noexcept {
        return pn_.c_str();
    }

    /**
     * @brief Return the string of Path
     * @return The string of Path
     */
    std::string String() const {
        return pn_;
    }

    /**
     * @brief Clear the Path object
     */
    void Clear() noexcept {
        pn_.clear();
    }

  private:
    string_type pn_;
};

/**
 * @brief Create a directory (attribute default 0755)
 * @param[in] p Dir path
 * @return Return true if created successfully; return false if the directory already exists
 * @throw FileSystemException
 * @par Example
 * @code
 * Path file_path("/root/main.cpp");
 * try {
 *     bool result = CreateDirectory(file_path);
 * } catch (FileSystemException& e) {
 *     ...
 * }
 * @endcode
 */
bool CreateDirectory(const Path& p);

/**
 * @brief Retrieve file type and permissions; If the file is a symbolic link, the retrieved file type and permissions
 * correspond to those of the target file it references.
 * @param[in] p File path
 * @return File status
 * @throw FileSystemException
 * @par Example
 * @code
 * Path file_path("/root/main.cpp");
 * try {
 *     FileStatus file_status = Status(file_path);
 * } catch (FileSystemException& e) {
 *     ...
 * }
 * @endcode
 */
FileStatus Status(const Path& p);

/**
 * @brief Retrieve file type and permissions; This function can retrieve the type and permissions of the symbolic link
 * file itself, distinct from its target file.
 * @param[in] p File path
 * @return File status
 * @throw FileSystemException
 * @par Example
 * @code
 * Path file_path("/root/main.cpp");
 * try {
 *     FileStatus file_status = SymlinkStatus(file_path);
 * } catch (FileSystemException& e) {
 *     ...
 * }
 * @endcode
 */
FileStatus SymlinkStatus(const Path& p);

/**
 * @brief Check if a file exists
 * @param[in] s File status
 * @return Return true if it exists; return false if it does not exist
 * @see ssgx::filesystem::Status(const Path& p)
 * @par Example
 * @code
 * Path file_path("/root/main.cpp");
 * try {
 *     FileStatus file_status = Status(file_path);
 * } catch (FileSystemException& e) {
 *     ...
 * }
 * bool result = Exists(file_status);
 * @endcode
 */
bool Exists(const FileStatus& s) noexcept;

/**
 * @brief Check if a file exists
 * @param[in] p File path
 * @return Return true if it exists; return false if it does not exist
 * @throw FileSystemException
 * @par Example
 * @code
 * Path file_path("/root/main.cpp");
 * try {
 *     bool result = Exists(file_path);
 * } catch (FileSystemException& e) {
 *     ...
 * }
 * @endcode
 */
bool Exists(const Path& p);

/**
 * @brief Remove a file
 * @param[in] p File path
 * @return Return true if removed successfully; return false if the file does not exist
 * @exception  FileSystemException
 * @par Example
 * @code
 * Path file_path("/root/main.cpp");
 * try {
 *     bool result = Remove(file_path);
 * } catch (FileSystemException& e) {
 *     ...
 * }
 * @endcode
 */
bool Remove(const Path& p);

/**
 * @brief Remove a protected file
 * @param[in] The protected file name
 * @return Return true if removed successfully; return false if the file does not exist
 * @exception  FileSystemException
 * @par Example
 * @code
 * Path file_path("/root/sealed_file");
 * try {
 *     bool result = RemoveProtectedFile(file_path);
 * } catch (FileSystemException& e) {
 *     ...
 * }
 * @endcode
 */
bool RemoveProtectedFile(const Path& file_name);

/**
 * @brief Check the size of the file
 * @param[in] p File path
 * @return File size
 * @throw FileSystemException
 * @par Example
 * @code
 * Path file_path("/root/main.cpp");
 * try {
 *     uint64_t result = FileSize(file_path);
 * } catch (FileSystemException& e) {
 *     ...
 * }
 * @endcode
 */
uintmax_t FileSize(const Path& p);

/**
 * @brief Check if the file is a directory
 * @param[in] s File status
 * @return Return true if the file type is a directory; otherwise return false
 * @see ::Status(const Path& p)
 * @par Example
 * @code
 * Path file_path("/root/main.cpp");
 * try {
 *     FileStatus file_status = Status(file_path);
 * } catch (FileSystemException& e) {
 *     ...
 * }
 * bool result = IsDirectory(file_status);
 * @endcode
 */
bool IsDirectory(const FileStatus& s) noexcept;

/**
 * @brief Check if the file is a directory
 * @param[in] p File path
 * @return Return true if the file type is a directory; otherwise return false
 * @throw FileSystemException
 * @par Example
 * @code
 * Path file_path("/root/main.cpp");
 * try {
 *     bool result = IsDirectory(file_path);
 * } catch (FileSystemException& e) {
 *     ...
 * }
 * @endcode
 */
bool IsDirectory(const Path& p);

/**
 * @brief If the path is a directory, check whether it is empty;
 *        If the path is a regular file, check whether its size is 0
 * @param[in] p Path
 * @return If the path is a directory: return true if the directory is empty; otherwise return false;  \n If the path is
 * a regular file: returns true if its size is 0; otherwise return false
 * @throw FileSystemException
 * @par Example
 * @code
 * Path file_path("/root");
 * try {
 *     bool result = IsEmpty(file_path);
 * } catch (FileSystemException& e) {
 *     ...
 * }
 * @endcode
 */
bool IsEmpty(const Path& p);

/**
 * @brief Check if the file is a regular file
 * @param[in] s File status
 * @return Return true if the file type is a regular file; otherwise return
 * false
 * @see ::Status(const Path& p)
 * @par Example
 * @code
 * Path file_path("/root/main.cpp");
 * try {
 *     FileStatus file_status = Status(file_path);
 * } catch (FileSystemException& e) {
 *     ...
 * }
 * bool result = IsRegularFile(file_status);
 * @endcode
 */
bool IsRegularFile(const FileStatus& s) noexcept;

/**
 * @brief Check if the file is a regular file
 * @param[in] p File path
 * @return Return true if the file type is a regular file; otherwise return false
 * @throw FileSystemException
 * @par Example
 * @code
 * Path file_path("/root/main.cpp");
 * try {
 *     bool result = IsRegularFile(file_path);
 * } catch (FileSystemException& e) {
 *     ...
 * }
 * @endcode
 */
bool IsRegularFile(const Path& p);

/**
 * @brief Exception about filesystem
 */
class FileSystemException : public std::exception {
  public:
    /**
     * @brief Construct a FileSystemException and initialized it
     */
    explicit FileSystemException(std::string err_msg) : err_msg_(std::move(err_msg)) {
    }

    /**
     * @brief Output exception message
     * @return Exception message
     */
    const char* what() const noexcept override {
        return err_msg_.c_str();
    }

  private:
    std::string err_msg_;
};

/**
 * @brief Read a plaintext file (file size <= 100 KB)
 */
class PlainFileReader {
  public:
    /**
     * @brief Construct a PlainFileReader and initialized it
     * @param[in] filepath File path
     */
    explicit PlainFileReader(std::string filepath) : file_path_(std::move(filepath)) {
    }

    /**
     * @brief Read a plaintext binary file, the max file size is 100KB.
     * @return Return the file content, in bytes.
     * @throws FileSystemException If the file is not open or a read error occurs.
     */
    std::vector<uint8_t> ReadAllBytes() const;

    /**
     * @brief Read a plaintext text file, the max file size is 100KB.
     * @return Return the file content, in string.
     * @throws FileSystemException If the file is not open or a read error occurs.
     */
    std::string ReadAllText() const;

  private:
    std::string file_path_;
};

/**
 * @brief Write a plaintext file (file size <= 100 KB)
 */
class PlainFileWriter {
  public:
    /**
     * @brief Construct a PlainFileWriter and initialized it
     * @param[in] filepath File path
     */
    explicit PlainFileWriter(std::string filepath) : file_path_(std::move(filepath)) {
    }

    /**
     * @brief Write data to a binary file in plaintext
     * @param[in] data data will be written to file, in bytes, the max size is 100KB.
     * @throws FileSystemException If the file is not open or a read error occurs.
     */
    void WriteAllBytes(const std::vector<uint8_t>& data);

    /**
     * @brief Write data to a text file in plaintext
     * @param[in] str data will be written to file, in string, the max size is 100KB.
     * @throws FileSystemException If the file is not open or a read error occurs.
     */
    void WriteAllText(const std::string& str);

  private:
    std::string file_path_;
};

/**
 * @brief A ProtectedFileReader for reading from the Intel SGX Protected File System.
 */
class ProtectedFileReader {
  private:
    SGX_FILE* file_;
    const char* file_name_;

  public:
    ProtectedFileReader(const ProtectedFileReader&) = delete;
    ProtectedFileReader& operator=(const ProtectedFileReader&) = delete;

    /**
     * @brief Opens a file for reading.
     * @param file_name The name of the file to open (must be a valid, non-null C-string).
     * @throws FileSystemException If the file cannot be opened.
     */
    explicit ProtectedFileReader(const char* file_name);
    ~ProtectedFileReader();

    /**
     * @brief Reads data from the file into the given buffer.
     * @param buffer Pointer to a valid memory location where data will be stored (must not be null).
     * @param size Maximum number of bytes to read (must be > 0).
     *        For small files, the recommended size is 4KB;
     *        For big files, the recommended size is 64KB;
     *        Don't set size >= 256KB.
     * @return Number of bytes actually read (0 if EOF is reached).
     * @throws FileSystemException If the file is not open or a read error occurs.
     */
    size_t Read(void* buffer, size_t size) const;

    /**
     * @brief Returns the current file position.
     * @return Current file position.
     * @throws FileSystemException If the file is not open or an error occurs.
     */
    int64_t Tell() const;

    /**
     * @brief Moves the file position.
     * @param offset Offset from the origin (can be negative).
     * @param origin One of SEEK_SET, SEEK_CUR, SEEK_END.
     * @throws FileSystemException If `origin` is invalid or seeking fails.
     */
    void Seek(int64_t offset, int origin) const;

    /**
     * @brief Closes the file.
     * @throws FileSystemException If closing fails.
     */
    void Close();
};

/**
 * @brief A ProtectedFileWriter for writing to the Intel SGX Protected File
 * System.
 */
class ProtectedFileWriter {
  private:
    SGX_FILE* file_;
    const char* file_name_;

  public:
    ProtectedFileWriter(const ProtectedFileWriter&) = delete;
    ProtectedFileWriter& operator=(const ProtectedFileWriter&) = delete;

    /**
     * @brief Opens a file for writing.
     * @param file_name The name of the file to create or overwrite (must be a valid, non-null C-string).
     * @param file_mode One of FileMode to identify the writing operation mode.
     * @param key_policy The key policy for sealing operations (default: SGX_KEYPOLICY_MRENCLAVE).
     * @throws FileSystemException If the file cannot be opened.
     */
    explicit ProtectedFileWriter(const char* file_name, FileMode file_mode = FileMode::CreateNew, uint16_t key_policy = SGX_KEYPOLICY_MRENCLAVE);

    ~ProtectedFileWriter();

    /**
     * @brief Writes data to the file.
     * @param data Pointer to the data to be written (must not be null).
     * @param size Size of the data in bytes (must be > 0).
     *        For small files, the recommended size is 4KB;
     *        For big files, the recommended size is 64KB;
     *        Don't set size >= 256KB.
     * @throws FileSystemException If writing fails.
     */
    void Write(const void* data, size_t size) const;

    /**
     * @brief Flushes any buffered data to disk.
     * @throws FileSystemException If flushing fails.
     */
    void Flush() const;

    /**
     * @brief Closes the file.
     * @throws FileSystemException If closing fails.
     */
    void Close();
};

}; // namespace filesystem_t
}; // namespace ssgx

#endif // SAFEHERON_SGX_TRUSTED_FILESTREAM_H
