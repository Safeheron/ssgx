#include <cstring>
#include <string>

#include "ssgx_config_t.h"
#include "ssgx_filesystem_t.h"
#include "ssgx_testframework_t.h"
#include "ssgx_utils_t.h"

using namespace ssgx::config_t;
using namespace ssgx::filesystem_t;

namespace {

const std::string kTestDataDir(TEST_DATA_DIR);
const std::string kSubDir = ".config_secret";

std::string TestPath(const std::string& filename) {
    Path p(kTestDataDir.c_str());
    p /= Path(kSubDir.c_str());
    p /= Path(filename.c_str());
    return p.String();
}

void EnsureSubDir() {
    Path p(kTestDataDir.c_str());
    p /= Path(kSubDir.c_str());
    if (!Exists(p))
        CreateDirectory(p);
}

void WriteFixture(const std::string& path, const std::string& content) {
    EnsureSubDir();
    PlainFileWriter w(path);
    w.WriteAllText(content);
}

std::string ReadAll(const std::string& path) {
    PlainFileReader r(path);
    return r.ReadAllText();
}

size_t CountOccurrences(const std::string& haystack, const std::string& needle) {
    if (needle.empty())
        return 0;
    size_t count = 0;
    size_t pos = 0;
    while ((pos = haystack.find(needle, pos)) != std::string::npos) {
        ++count;
        pos += needle.size();
    }
    return count;
}

} // namespace

TEST(ConfigSecretSuite, FirstTimeSealAndReadback) {
    const std::string path = TestPath("simple.toml");
    WriteFixture(path,
                 "[database]\n"
                 "host = \"localhost\"\n"
                 "password.secret = \"my_password\"\n");

    TomlConfig cfg;
    ASSERT_TRUE(cfg.LoadFile(path.c_str()));

    auto pwd = cfg.GetSecretString("database", "password");
    ASSERT_TRUE(pwd.has_value());
    ASSERT_EQ(pwd.value(), "my_password");
    ASSERT_TRUE(cfg.SaveFile());

    const std::string content = ReadAll(path);
    ASSERT_EQ(content.find(".secret"), std::string::npos);
    ASSERT_NE(content.find("password.sealed"), std::string::npos);
    ASSERT_NE(content.find("ssgxcfg.v1:"), std::string::npos);
    // unrelated lines preserved
    ASSERT_NE(content.find("host = \"localhost\""), std::string::npos);
    ASSERT_NE(content.find("[database]"), std::string::npos);
}

TEST(ConfigSecretSuite, SteadyStateReadAfterSeal) {
    const std::string path = TestPath("steady.toml");
    WriteFixture(path,
                 "[database]\n"
                 "password.secret = \"steady_pwd\"\n");

    {
        TomlConfig cfg1;
        ASSERT_TRUE(cfg1.LoadFile(path.c_str()));
        auto v1 = cfg1.GetSecretString("database", "password");
        ASSERT_TRUE(v1.has_value());
        ASSERT_EQ(v1.value(), "steady_pwd");
    }
    // Re-open the same (now-sealed) file with a fresh TomlConfig.
    {
        TomlConfig cfg2;
        ASSERT_TRUE(cfg2.LoadFile(path.c_str()));
        auto v2 = cfg2.GetSecretString("database", "password");
        ASSERT_TRUE(v2.has_value());
        ASSERT_EQ(v2.value(), "steady_pwd");
    }
}

TEST(ConfigSecretSuite, FormatAndCommentPreservation) {
    const std::string path = TestPath("format.toml");
    const std::string before =
        "# Top-level comment\n"
        "\n"
        "[server]\n"
        "host = \"example.com\"   # important\n"
        "port = 8080\n"
        "\n"
        "[database]\n"
        "name = \"prod_db\"\n"
        "password.secret = \"hush\"   # the password\n"
        "max_connections = 100\n";
    WriteFixture(path, before);

    TomlConfig cfg;
    ASSERT_TRUE(cfg.LoadFile(path.c_str()));
    auto v = cfg.GetSecretString("database", "password");
    ASSERT_TRUE(v.has_value());
    ASSERT_EQ(v.value(), "hush");
    ASSERT_TRUE(cfg.SaveFile());

    const std::string after = ReadAll(path);
    ASSERT_NE(after.find("# Top-level comment"), std::string::npos);
    ASSERT_NE(after.find("[server]"), std::string::npos);
    // toml11 v4 moves suffix comments to the preceding line of the next key,
    // so only check that both the value and the comment exist somewhere in the output.
    ASSERT_NE(after.find("host = \"example.com\""), std::string::npos);
    ASSERT_NE(after.find("# important"), std::string::npos);
    ASSERT_NE(after.find("name = \"prod_db\""), std::string::npos);
    ASSERT_NE(after.find("max_connections = 100"), std::string::npos);
    // The trailing comment on the secret line is preserved
    ASSERT_NE(after.find("# the password"), std::string::npos);
    // .secret is gone, .sealed is in
    ASSERT_EQ(after.find(".secret"), std::string::npos);
    ASSERT_NE(after.find("password.sealed"), std::string::npos);
}

TEST(ConfigSecretSuite, MultipleSecretsInOneFile) {
    const std::string path = TestPath("multi.toml");
    WriteFixture(path,
                 "[a]\n"
                 "key.secret = \"alpha\"\n"
                 "[b]\n"
                 "key.secret = \"bravo\"\n"
                 "[c]\n"
                 "key.secret = \"charlie\"\n");

    TomlConfig cfg;
    ASSERT_TRUE(cfg.LoadFile(path.c_str()));
    ASSERT_EQ(cfg.GetSecretString("a", "key").value_or(""), "alpha");
    ASSERT_EQ(cfg.GetSecretString("b", "key").value_or(""), "bravo");
    ASSERT_EQ(cfg.GetSecretString("c", "key").value_or(""), "charlie");
    ASSERT_TRUE(cfg.SaveFile());

    const std::string after = ReadAll(path);
    ASSERT_EQ(after.find(".secret"), std::string::npos);
    ASSERT_EQ(CountOccurrences(after, "key.sealed"), 3u);
}

TEST(ConfigSecretSuite, AadBindsToFieldPath) {
    // Step 1: seal a value at path "alpha"
    const std::string src_path = TestPath("aad_src.toml");
    WriteFixture(src_path, "alpha.secret = \"value_alpha\"\n");
    {
        TomlConfig cfg;
        ASSERT_TRUE(cfg.LoadFile(src_path.c_str()));
        ASSERT_EQ(cfg.GetSecretString("alpha").value_or(""), "value_alpha");
    }

    // Step 2: extract the sealed value text
    const std::string src_after = ReadAll(src_path);
    const std::string lead = "alpha.sealed = \"";
    const auto p1 = src_after.find(lead);
    ASSERT_NE(p1, std::string::npos);
    const auto start = p1 + lead.size();
    const auto end = src_after.find('"', start);
    ASSERT_NE(end, std::string::npos);
    const std::string sealed_alpha = src_after.substr(start, end - start);

    // Step 3: paste alpha's sealed blob under a different path "bravo"
    const std::string tgt_path = TestPath("aad_tgt.toml");
    WriteFixture(tgt_path, "bravo.sealed = \"" + sealed_alpha + "\"\n");

    // Step 4: GetSecretString("bravo") should fail because AAD ("alpha") differs from path ("bravo")
    TomlConfig cfg;
    ASSERT_TRUE(cfg.LoadFile(tgt_path.c_str()));
    auto v = cfg.GetSecretString("bravo");
    ASSERT_FALSE(v.has_value());
    const std::string err = cfg.GetLastErrorMsg();
    ASSERT_NE(err.find("unseal"), std::string::npos);
}

TEST(ConfigSecretSuite, UnknownSealedPrefix) {
    const std::string path = TestPath("bad_prefix.toml");
    WriteFixture(path, "x.sealed = \"unknown_prefix:something\"\n");

    TomlConfig cfg;
    ASSERT_TRUE(cfg.LoadFile(path.c_str()));
    auto v = cfg.GetSecretString("x");
    ASSERT_FALSE(v.has_value());
    ASSERT_NE(cfg.GetLastErrorMsg().find("unknown sealed format prefix"), std::string::npos);
}

TEST(ConfigSecretSuite, InvalidBase64) {
    const std::string path = TestPath("bad_b64.toml");
    WriteFixture(path, "x.sealed = \"ssgxcfg.v1:!!!not-base64!!!\"\n");

    TomlConfig cfg;
    ASSERT_TRUE(cfg.LoadFile(path.c_str()));
    auto v = cfg.GetSecretString("x");
    ASSERT_FALSE(v.has_value());
    // Either base64 decode failed, or decode succeeded into garbage and unseal failed.
    const std::string err = cfg.GetLastErrorMsg();
    ASSERT_FALSE(err.empty());
}

TEST(ConfigSecretSuite, PlainFieldRejected) {
    const std::string path = TestPath("plain.toml");
    WriteFixture(path,
                 "[database]\n"
                 "password = \"plain_value\"\n");

    TomlConfig cfg;
    ASSERT_TRUE(cfg.LoadFile(path.c_str()));
    auto v = cfg.GetSecretString("database", "password");
    ASSERT_FALSE(v.has_value());
    ASSERT_NE(cfg.GetLastErrorMsg().find("not annotated as a secret"), std::string::npos);
}

TEST(ConfigSecretSuite, MultilineStringSealed) {
    const std::string path = TestPath("multiline.toml");
    // toml11 parses multiline basic strings as plain std::string; per TOML spec the first newline is trimmed.
    WriteFixture(path,
                 "x.secret = \"\"\"\n"
                 "multi\n"
                 "line\n"
                 "\"\"\"\n");

    TomlConfig cfg;
    ASSERT_TRUE(cfg.LoadFile(path.c_str()));
    auto v = cfg.GetSecretString("x");
    ASSERT_TRUE(v.has_value());
    ASSERT_EQ(v.value(), "multi\nline\n");
}

TEST(ConfigSecretSuite, EmptySecretRefused) {
    const std::string path = TestPath("empty.toml");
    WriteFixture(path, "x.secret = \"\"\n");

    TomlConfig cfg;
    ASSERT_TRUE(cfg.LoadFile(path.c_str()));
    auto v = cfg.GetSecretString("x");
    ASSERT_FALSE(v.has_value());
    ASSERT_NE(cfg.GetLastErrorMsg().find("empty"), std::string::npos);
}

// Verify that calling GetSecretString twice on the same TomlConfig instance works:
// the first call seals the value and updates the in-memory context in-place;
// the second call must find the sealed node in that same context and unseal it.
TEST(ConfigSecretSuite, SameInstanceDoubleRead) {
    const std::string path = TestPath("same_instance.toml");
    WriteFixture(path,
                 "[srv]\n"
                 "token.secret = \"abc123\"\n");

    TomlConfig cfg;
    ASSERT_TRUE(cfg.LoadFile(path.c_str()));

    auto v1 = cfg.GetSecretString("srv", "token");
    ASSERT_TRUE(v1.has_value());
    ASSERT_EQ(v1.value(), "abc123");

    auto v2 = cfg.GetSecretString("srv", "token");
    ASSERT_TRUE(v2.has_value());
    ASSERT_EQ(v2.value(), "abc123");
}

// Verify that non-secret keys keep their original relative order in the file
// after a secret is sealed (ordered_value preserves insertion order).
TEST(ConfigSecretSuite, KeyOrderPreservedAfterSeal) {
    const std::string path = TestPath("order.toml");
    WriteFixture(path,
                 "[cfg]\n"
                 "first = \"a\"\n"
                 "middle.secret = \"secret_val\"\n"
                 "last = \"z\"\n");

    TomlConfig cfg;
    ASSERT_TRUE(cfg.LoadFile(path.c_str()));
    ASSERT_TRUE(cfg.GetSecretString("cfg", "middle").has_value());
    ASSERT_TRUE(cfg.SaveFile());

    const std::string after = ReadAll(path);
    const auto pos_first  = after.find("first");
    const auto pos_middle = after.find("middle.sealed");
    const auto pos_last   = after.find("last");
    ASSERT_NE(pos_first,  std::string::npos);
    ASSERT_NE(pos_middle, std::string::npos);
    ASSERT_NE(pos_last,   std::string::npos);
    ASSERT_LT(pos_first,  pos_middle);
    ASSERT_LT(pos_middle, pos_last);
}
