#include <string.h>

#include "ssgx_exception_t.h"
#include "ssgx_filesystem_t.h"
#include "ssgx_filesystem_t_enum.h"
#include "ssgx_testframework_t.h"
#include "ssgx_utils_t.h"

#include "Enclave_t.h"

using namespace ssgx::filesystem_t;

/**
 *  !!!IMPORTANT NOTICE!!!
 * In order to run this test case correctly, before running this test case, please make sure
 *  1. folder ${TEST_DATA_DIR}/.filesystem doesn't exist, or
 *  2. it is empty.
 *
 *  Otherwise, the test case cannot run as designed.
 */

const static std::string test_dir(TEST_DATA_DIR);
const static std::string test_sub_dir = ".filesystem";
const static std::string plain_file_name = "test_plain_file";
const static std::string protected_file_name = "test_protected_file";
const static std::string protected_meta_file_name = "test_protected_file.pfsmeta";

const static std::string plain_file_content = "The is a plain text file!";
const static std::string protected_file_content = "The is a protected text file! File content is sealed by seal key.";

TEST(FilesystemTestSuite, CreateDirectory) {
    Path new_dir(test_dir.c_str());
    new_dir /= Path(test_sub_dir.c_str());
    ASSERT_EQ(new_dir.String(), test_dir + "/" + test_sub_dir);

    if (Exists(new_dir)) {
        ASSERT_FALSE(CreateDirectory(new_dir));
    } else {
        ASSERT_TRUE(CreateDirectory(new_dir));
    }
    ASSERT_TRUE(IsDirectory(new_dir));
}

TEST(FilesystemTestSuite, FileStatus) {
    Path test_file(test_dir.c_str());
    test_file /= Path(test_sub_dir.c_str());
    test_file /= Path("test_file");

    const std::vector<uint8_t> test_data = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    PlainFileWriter writer(test_file.String());
    ASSERT_NO_THROW(writer.WriteAllBytes(test_data));

    const FileStatus fs = Status(test_file);
    ASSERT_TRUE(Exists(fs));
    ASSERT_TRUE(IsRegularFile(fs));

    ASSERT_TRUE(Remove(test_file));
}

TEST(FilesystemTestSuite, SymlinkStatus) {
    const std::string test_data = "This is a regular file.";

    Path regular_file(test_dir.c_str());
    regular_file /= Path(test_sub_dir.c_str());
    regular_file /= Path("regular_file");
    PlainFileWriter writer(regular_file.String());
    ASSERT_NO_THROW(writer.WriteAllText(test_data));

    int ret = 0;
    Path link_file(test_dir.c_str());
    link_file /= Path(test_sub_dir.c_str());
    link_file /= Path("link_file");
    ASSERT_TRUE(ocall_symlink_file(&ret, regular_file.c_str(), link_file.c_str()) == SGX_SUCCESS);
    ASSERT_TRUE(ret == 0);

    FileStatus file_status_1 = Status(link_file);
    ASSERT_TRUE(Exists(file_status_1));
    ASSERT_TRUE(IsRegularFile(file_status_1));

    FileStatus file_status_2 = SymlinkStatus(link_file);
    ASSERT_TRUE(Exists(file_status_2));
    ASSERT_FALSE(IsRegularFile(file_status_2));

    ASSERT_EQ(FileSize(link_file), test_data.length());

    ASSERT_TRUE(Remove(link_file));
    ASSERT_TRUE(Remove(regular_file));
}

TEST(FilesystemTestSuite, FileExists) {
    Path working_dir(test_dir.c_str());
    working_dir /= Path(test_sub_dir.c_str());
    ASSERT_TRUE(Exists(working_dir));

    Path test_file = working_dir / Path("test_file");
    const std::vector<uint8_t> test_data = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    PlainFileWriter writer(test_file.String());
    ASSERT_NO_THROW(writer.WriteAllBytes(test_data));
    ASSERT_TRUE(Exists(test_file));

    ASSERT_TRUE(Remove(test_file));
}

TEST(FilesystemTestSuite, RemoveFile) {
    Path test_file(test_dir.c_str());
    test_file /= Path(test_sub_dir.c_str());
    test_file /= Path("test_file");

    const std::vector<uint8_t> test_data = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    PlainFileWriter writer(test_file.String());
    ASSERT_NO_THROW(writer.WriteAllBytes(test_data));
    ASSERT_TRUE(Exists(test_file));

    FileStatus fs = Status(test_file);
    ASSERT_TRUE(Exists(fs));

    ASSERT_TRUE(Remove(test_file));
    ASSERT_FALSE(Exists(test_file));

    fs = Status(test_file);
    ASSERT_FALSE(Exists(fs));
}

TEST(FilesystemTestSuite, FileSize) {
    Path test_file(test_dir.c_str());
    test_file /= Path(test_sub_dir.c_str());
    test_file /= Path("test_file");

    const std::vector<uint8_t> test_data = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    PlainFileWriter writer(test_file.String());
    ASSERT_NO_THROW(writer.WriteAllBytes(test_data));
    ASSERT_EQ(FileSize(test_file), test_data.size());
    ASSERT_TRUE(Remove(test_file));
}

TEST(FilesystemTestSuite, IsDirectory) {
    Path working_dir(test_dir.c_str());
    ASSERT_TRUE(IsDirectory(working_dir));

    working_dir /= Path(test_sub_dir.c_str());
    ASSERT_TRUE(IsDirectory(working_dir));

    Path test_file = working_dir / Path("test_file");
    const std::vector<uint8_t> test_data = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    PlainFileWriter writer(test_file.String());
    ASSERT_NO_THROW(writer.WriteAllBytes(test_data));
    ASSERT_FALSE(IsDirectory(test_file));
    ASSERT_TRUE(Remove(test_file));
}

TEST(FilesystemTestSuite, IsEmpty) {
    Path working_dir(test_dir.c_str());
    ASSERT_FALSE(IsEmpty(working_dir));

    working_dir /= Path(test_sub_dir.c_str());
    ASSERT_TRUE(IsEmpty(working_dir));

    Path test_file = working_dir / Path("test_file");

    const std::vector<uint8_t> empty_data;
    PlainFileWriter writer1(test_file.String());
    ASSERT_NO_THROW(writer1.WriteAllBytes(empty_data));
    ASSERT_TRUE(IsEmpty(test_file));
    ASSERT_TRUE(Remove(test_file));

    const std::string empty_str;
    PlainFileWriter writer2(test_file.String());
    ASSERT_NO_THROW(writer2.WriteAllText(empty_str));
    ASSERT_TRUE(IsEmpty(test_file));
    ASSERT_TRUE(Remove(test_file));
}

TEST(FilesystemTestSuite, IsRegularFile) {
    const std::string test_data = "This is a regular file.";

    Path regular_file(test_dir.c_str());
    regular_file /= Path(test_sub_dir.c_str());
    regular_file /= Path("regular_file");
    PlainFileWriter writer(regular_file.String());
    ASSERT_NO_THROW(writer.WriteAllText(test_data));
    FileStatus fs = Status(regular_file);
    ASSERT_TRUE(IsRegularFile(regular_file));
    ASSERT_TRUE(IsRegularFile(fs));

    int ret = 0;
    Path link_file(test_dir.c_str());
    link_file /= Path(test_sub_dir.c_str());
    link_file /= Path("link_file");
    ASSERT_TRUE(ocall_symlink_file(&ret, regular_file.c_str(), link_file.c_str()) == SGX_SUCCESS);
    ASSERT_TRUE(ret == 0);
    fs = SymlinkStatus(link_file);
    ASSERT_TRUE(IsRegularFile(link_file));
    ASSERT_FALSE(IsRegularFile(fs));

    ASSERT_TRUE(Remove(link_file));
    ASSERT_TRUE(Remove(regular_file));
}

TEST(FilesystemTestSuite, PlainFileWriter_Binary) {
    Path plain_file(test_dir.c_str());
    plain_file /= Path(test_sub_dir.c_str());
    plain_file /= Path(plain_file_name.c_str());

    const std::vector<uint8_t> test_data(plain_file_content.begin(), plain_file_content.end());
    PlainFileWriter writer(plain_file.String());
    ASSERT_NO_THROW(writer.WriteAllBytes(test_data));
}

TEST(FilesystemTestSuite, PlainFileReader_Binary) {
    Path plain_file(test_dir.c_str());
    plain_file /= Path(test_sub_dir.c_str());
    plain_file /= Path(plain_file_name.c_str());

    std::vector<uint8_t> file_content;
    PlainFileReader reader(plain_file.String());
    ASSERT_NO_THROW(file_content = reader.ReadAllBytes());
    std::string test_data = std::string(file_content.begin(), file_content.end());
    ASSERT_EQ(test_data, plain_file_content);

    ASSERT_TRUE(Remove(plain_file));
}

TEST(FilesystemTestSuite, PlainFileWriter_Text) {
    Path plain_file(test_dir.c_str());
    plain_file /= Path(test_sub_dir.c_str());
    plain_file /= Path(plain_file_name.c_str());

    PlainFileWriter writer(plain_file.String());
    ASSERT_NO_THROW(writer.WriteAllText(plain_file_content));
}

TEST(FilesystemTestSuite, PlainFileReader_Text) {
    Path plain_file(test_dir.c_str());
    plain_file /= Path(test_sub_dir.c_str());
    plain_file /= Path(plain_file_name.c_str());

    std::string test_data;
    PlainFileReader reader(plain_file.String());
    ASSERT_NO_THROW(test_data = reader.ReadAllText());
    ASSERT_EQ(test_data, plain_file_content);

    ASSERT_TRUE(Remove(plain_file));
}

TEST(FilesystemTestSuite, PlainFileExceedLimited) {
    Path plain_file(test_dir.c_str());
    plain_file /= Path(test_sub_dir.c_str());
    plain_file /= Path(plain_file_name.c_str());

    // The max supported size is 100K
    std::vector<uint8_t> file_content(100 * 1024 + 1, 0);
    PlainFileWriter writer(plain_file.String());
    ASSERT_THROW(writer.WriteAllBytes(file_content), FileSystemException);
}

void write_protected_file(const Path& file_name, FileMode file_mode, uint16_t key_policy, const std::string& content) {
    // The size of data to read/write each time.
    // For small files, the recommended size is 4KB;
    // For big files, the recommended size is 64KB;
    // Don't set SIZE_PER_TIME > 256KB.
    constexpr uint32_t SIZE_PER_TIME = 4 * 1024; // 4KB each time

    size_t written_size = 0;
    size_t left_size = content.size();
    ASSERT_NO_THROW({
        ProtectedFileWriter writer(file_name.String().c_str(), file_mode, key_policy);
        while (left_size > 0) {
            size_t write_size = (left_size > SIZE_PER_TIME) ? SIZE_PER_TIME : left_size;
            writer.Write(content.c_str() + written_size, write_size);
            written_size += write_size;
            left_size -= write_size;
        }
        writer.Close();
    });
}

void read_protected_file(const Path& file_name, std::string& content) {
    // The size of data to read/write each time.
    // For small files, the recommended size is 4KB;
    // For big files, the recommended size is 64KB;
    // Don't set SIZE_PER_TIME > 256KB.
    constexpr uint32_t SIZE_PER_TIME = 4 * 1024; // 4KB each time
    uint8_t buffer[SIZE_PER_TIME] = {0};

    content.clear();
    size_t read_size = 0;
    ASSERT_NO_THROW({
        ProtectedFileReader reader(file_name.String().c_str());
        while ((read_size = reader.Read(buffer, SIZE_PER_TIME)) > 0) {
            content.append((char*)buffer, read_size);
        }
        reader.Close();
    });
}

TEST(FilesystemTestSuite, ProtectedFile_CreateNew) {
    Path working_dir(test_dir.c_str());
    working_dir /= Path(test_sub_dir.c_str());
    Path protected_file = working_dir / Path(protected_file_name.c_str());
    Path protected_meta_file = working_dir / Path(protected_meta_file_name.c_str());
    std::string content;

    ASSERT_NO_THROW(RemoveProtectedFile(protected_file));

    write_protected_file(protected_file, FileMode::CreateNew, SGX_KEYPOLICY_MRENCLAVE, protected_file_content);
    ASSERT_TRUE(Exists(protected_file));
    ASSERT_TRUE(Exists(protected_meta_file));
    read_protected_file(protected_file, content);
    ASSERT_EQ(content, protected_file_content);

    // In CreateNew mode, throw exception if the file exists.
    ASSERT_THROW(
        ProtectedFileWriter writer(protected_file.String().c_str(), FileMode::CreateNew, SGX_KEYPOLICY_MRENCLAVE),
        FileSystemException
        );
    ASSERT_NO_THROW(RemoveProtectedFile(protected_file));

    write_protected_file(protected_file, FileMode::CreateNew, SGX_KEYPOLICY_MRSIGNER, protected_file_content);
    ASSERT_TRUE(Exists(protected_file));
    ASSERT_TRUE(Exists(protected_meta_file));
    read_protected_file(protected_file, content);
    ASSERT_EQ(content, protected_file_content);
}

TEST(FilesystemTestSuite, ProtectedFile_OpenOrCreate) {
    Path working_dir(test_dir.c_str());
    working_dir /= Path(test_sub_dir.c_str());
    Path protected_file = working_dir / Path(protected_file_name.c_str());
    Path protected_meta_file = working_dir / Path(protected_meta_file_name.c_str());
    std::string content;

    ASSERT_NO_THROW(RemoveProtectedFile(protected_file));

    write_protected_file(protected_file, FileMode::OpenOrCreate, SGX_KEYPOLICY_MRENCLAVE, protected_file_content);
    ASSERT_TRUE(Exists(protected_file));
    ASSERT_TRUE(Exists(protected_meta_file));
    read_protected_file(protected_file, content);
    ASSERT_EQ(content, protected_file_content);

    // In OpenOrCreate mode, file content will be overwritten if the file exists.
    write_protected_file(protected_file, FileMode::OpenOrCreate, SGX_KEYPOLICY_MRSIGNER, protected_file_content);
    ASSERT_TRUE(Exists(protected_file));
    ASSERT_TRUE(Exists(protected_meta_file));
    read_protected_file(protected_file, content);
    ASSERT_EQ(content, protected_file_content);
}

TEST(FilesystemTestSuite, ProtectedFile_Append) {
    Path working_dir(test_dir.c_str());
    working_dir /= Path(test_sub_dir.c_str());
    Path protected_file = working_dir / Path(protected_file_name.c_str());
    Path protected_meta_file = working_dir / Path(protected_meta_file_name.c_str());
    std::string content;

    ASSERT_NO_THROW(RemoveProtectedFile(protected_file));
    write_protected_file(protected_file, FileMode::Append, SGX_KEYPOLICY_MRENCLAVE, protected_file_content);
    ASSERT_TRUE(Exists(protected_file));
    ASSERT_TRUE(Exists(protected_meta_file));
    read_protected_file(protected_file, content);
    ASSERT_EQ(content, protected_file_content);

    write_protected_file(protected_file, FileMode::Append, SGX_KEYPOLICY_MRENCLAVE, protected_file_content);
    ASSERT_TRUE(Exists(protected_file));
    ASSERT_TRUE(Exists(protected_meta_file));

    // In Append mode, file content will be appended if the file exists.
    read_protected_file(protected_file, content);
    ASSERT_EQ(content, protected_file_content + protected_file_content);

    // Throw FileSystemException exception if try to append a protected file with different seal key policy.
    ASSERT_THROW(
        ProtectedFileWriter writer(protected_file.String().c_str(), FileMode::Append, SGX_KEYPOLICY_MRSIGNER),
        FileSystemException
        );
    ASSERT_NO_THROW(RemoveProtectedFile(protected_file));
}

TEST(FilesystemTestSuite, ProtectedFile_LegacyFile) {
    Path working_dir(test_dir.c_str());
    working_dir /= Path(test_sub_dir.c_str());
    Path protected_file = working_dir / Path(protected_file_name.c_str());
    Path protected_meta_file = working_dir / Path(protected_meta_file_name.c_str());
    std::string content;

    ASSERT_NO_THROW(RemoveProtectedFile(protected_file));

    write_protected_file(protected_file, FileMode::CreateNew, SGX_KEYPOLICY_MRSIGNER, protected_file_content);
    ASSERT_TRUE(Exists(protected_file));
    ASSERT_TRUE(Exists(protected_meta_file));
    read_protected_file(protected_file, content);
    ASSERT_EQ(content, protected_file_content);

    // The existing file will be overwritten
    write_protected_file(protected_file, FileMode::OpenOrCreate, SGX_KEYPOLICY_MRSIGNER, protected_file_content);
    ASSERT_TRUE(Exists(protected_file));
    ASSERT_TRUE(Exists(protected_meta_file));
    read_protected_file(protected_file, content);
    ASSERT_EQ(content, protected_file_content);

    ASSERT_NO_THROW(RemoveProtectedFile(protected_file));
    // A new file should be created
    write_protected_file(protected_file, FileMode::OpenOrCreate, SGX_KEYPOLICY_MRSIGNER, protected_file_content);
    ASSERT_TRUE(Exists(protected_file));
    ASSERT_TRUE(Exists(protected_meta_file));
    read_protected_file(protected_file, content);
    ASSERT_EQ(content, protected_file_content);

    // New content will be appended to the file
    write_protected_file(protected_file, FileMode::Append, SGX_KEYPOLICY_MRSIGNER, protected_file_content);
    ASSERT_TRUE(Exists(protected_file));
    ASSERT_TRUE(Exists(protected_meta_file));
    read_protected_file(protected_file, content);
    ASSERT_EQ(content, protected_file_content + protected_file_content);

    // No metadata file, reading and writing operation will succeed when key_policy is SGX_KEYPOLICY_MRSIGNER
    ASSERT_NO_THROW(Remove(protected_meta_file));
    read_protected_file(protected_file, content);
    ASSERT_EQ(content, protected_file_content + protected_file_content);
    write_protected_file(protected_file, FileMode::Append, SGX_KEYPOLICY_MRSIGNER, protected_file_content);
    ASSERT_TRUE(Exists(protected_file));
    ASSERT_TRUE(Exists(protected_meta_file));
    read_protected_file(protected_file, content);
    ASSERT_EQ(content, protected_file_content + protected_file_content + protected_file_content);

    ASSERT_NO_THROW(RemoveProtectedFile(protected_file));

    // The protected file must be compatible with Intel SGX API: sgx_fopen_auto_key()
    // 1. Use ProtectedFileWriter to create a file with SGX_KEYPOLICY_MRSIGNER, then use sgx_fopen_auto_key()
    // to open it and read the file content.
    write_protected_file(protected_file, FileMode::CreateNew, SGX_KEYPOLICY_MRSIGNER, protected_file_content);
    ASSERT_TRUE(Exists(protected_file));
    ASSERT_TRUE(Exists(protected_meta_file));
    SGX_FILE* file = sgx_fopen_auto_key(protected_file.c_str(), "r");
    ASSERT_TRUE(file != nullptr);
    char file_content[256] = {0};
    size_t size = sgx_fread(file_content, 1, sizeof(file_content), file);
    sgx_fclose(file);
    ASSERT_EQ(size, protected_file_content.size());
    ASSERT_EQ(file_content, protected_file_content);

    // The protected file must be compatible with Intel SGX API: sgx_fopen_auto_key()
    // 2. Use sgx_fopen_auto_key() to create a file, then use ProtectedFileReader to open it and read its content.
    file = sgx_fopen_auto_key(protected_file.c_str(), "w");
    ASSERT_TRUE(file != nullptr);
    size = sgx_fwrite(protected_file_content.c_str(), 1, protected_file_content.size(), file);
    sgx_fclose(file);
    ASSERT_EQ(size, protected_file_content.size());
    read_protected_file(protected_file, content);
    ASSERT_EQ(content, protected_file_content);
}

TEST(FilesystemTestSuite, ProtectedFile_Seek) {
    Path working_dir(test_dir.c_str());
    working_dir /= Path(test_sub_dir.c_str());
    Path protected_file = working_dir / Path(protected_file_name.c_str());
    Path protected_meta_file = working_dir / Path(protected_meta_file_name.c_str());
    std::string content;

    if (Exists(protected_file)) {
        ASSERT_TRUE(Remove(protected_file));
    }
    if (Exists(protected_meta_file)) {
        ASSERT_TRUE(Remove(protected_meta_file));
    }

    write_protected_file(protected_file, FileMode::OpenOrCreate, SGX_KEYPOLICY_MRSIGNER, protected_file_content);
    ASSERT_TRUE(Exists(protected_file));
    ASSERT_TRUE(Exists(protected_meta_file));
    ASSERT_TRUE(Remove(protected_meta_file));

    uint8_t buffer[4096] = {0};
    ProtectedFileReader reader(protected_file.String().c_str());
    reader.Seek(1, SEEK_SET);
    ASSERT_EQ(1, reader.Tell());
    size_t read_size = reader.Read(buffer, 4096);
    content.append((char*)buffer, read_size);
    ASSERT_EQ(content, protected_file_content.substr(1, protected_file_content.size()-1));
}

TEST(FilesystemTestSuite, ProtectedFile_BigFile) {
    Path working_dir(test_dir.c_str());
    working_dir /= Path(test_sub_dir.c_str());
    Path protected_file = working_dir / Path(protected_file_name.c_str());
    Path protected_meta_file = working_dir / Path(protected_meta_file_name.c_str());

    std::string input_content;
    std::string content;

    if (Exists(protected_file)) {
        ASSERT_TRUE(Remove(protected_file));
    }
    if (Exists(protected_meta_file)) {
        ASSERT_TRUE(Remove(protected_meta_file));
    }

    for (int i = 0; i < 1000000; i++) {
        input_content.append(protected_file_content);
    }

    write_protected_file(protected_file, FileMode::OpenOrCreate, SGX_KEYPOLICY_MRSIGNER | SGX_KEYPOLICY_ISVFAMILYID, input_content);
    ASSERT_TRUE(Exists(protected_file));
    ASSERT_TRUE(Exists(protected_meta_file));

    read_protected_file(protected_file, content);
    ASSERT_EQ(content, input_content);
}

TEST(FilesystemTestSuite, ProtectedFile_MRSINGER_and_MRENCLAVE) {
    Path working_dir(test_dir.c_str());
    working_dir /= Path(test_sub_dir.c_str());
    Path protected_file = working_dir / Path(protected_file_name.c_str());
    Path protected_meta_file = working_dir / Path(protected_meta_file_name.c_str());
    std::string content;

    if (Exists(protected_file)) {
        ASSERT_TRUE(Remove(protected_file));
    }
    if (Exists(protected_meta_file)) {
        ASSERT_TRUE(Remove(protected_meta_file));
    }

    write_protected_file(protected_file, FileMode::CreateNew, SGX_KEYPOLICY_MRSIGNER | SGX_KEYPOLICY_MRENCLAVE, protected_file_content);
    ASSERT_TRUE(Exists(protected_file));
    ASSERT_TRUE(Exists(protected_meta_file));
    read_protected_file(protected_file, content);
    ASSERT_EQ(content, protected_file_content);
}

TEST(FilesystemTestSuite, ProtectedFile_ALL) {
    Path working_dir(test_dir.c_str());
    working_dir /= Path(test_sub_dir.c_str());
    Path protected_file = working_dir / Path(protected_file_name.c_str());
    Path protected_meta_file = working_dir / Path(protected_meta_file_name.c_str());
    std::string content;

    if (Exists(protected_file)) {
        ASSERT_TRUE(Remove(protected_file));
    }
    if (Exists(protected_meta_file)) {
        ASSERT_TRUE(Remove(protected_meta_file));
    }

    // SGX_KEYPOLICY_NOISVPRODID | SGX_KEYPOLICY_CONFIGID | SGX_KEYPOLICY_ISVFAMILYID | SGX_KEYPOLICY_ISVEXTPRODID
    // These parameters can only be used when the enclave enables KSS feature.
    write_protected_file(protected_file, FileMode::CreateNew, SGX_KEYPOLICY_MRSIGNER | SGX_KEYPOLICY_MRENCLAVE | SGX_KEYPOLICY_NOISVPRODID | SGX_KEYPOLICY_CONFIGID | SGX_KEYPOLICY_ISVFAMILYID | SGX_KEYPOLICY_ISVEXTPRODID, protected_file_content);
    ASSERT_TRUE(Exists(protected_file));
    ASSERT_TRUE(Exists(protected_meta_file));
    read_protected_file(protected_file, content);
    ASSERT_EQ(content, protected_file_content);
}

TEST(FilesystemTestSuite, RemoveDirectory) {
    Path working_dir(test_dir.c_str());
    working_dir /= Path(test_sub_dir.c_str());
    Path plain_file = working_dir / Path(plain_file_name.c_str());
    Path protected_file = working_dir / Path(protected_file_name.c_str());
    Path protected_meta_file = working_dir / Path(protected_meta_file_name.c_str());

    if (Exists(plain_file)) {
        ASSERT_TRUE(Remove(plain_file));
    }
    if (Exists(protected_file)) {
        ASSERT_TRUE(Remove(protected_file));
    }
    if (Exists(protected_meta_file)) {
        ASSERT_TRUE(Remove(protected_meta_file));
    }

    ASSERT_TRUE(Remove(working_dir));
}