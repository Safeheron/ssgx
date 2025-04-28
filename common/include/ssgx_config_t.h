#ifndef SAFEHERON_SGX_TRUSTED_CONFIG_H
#define SAFEHERON_SGX_TRUSTED_CONFIG_H

#include <optional>
#include <string>
#include <vector>

namespace ssgx {
/**
 * @brief This module is designed to be able to directly read configuration file data in enclave.
 * @details The configuration file should be written in the [TOML v1.0.0](https://toml.io/en/v1.0.0) format.
 *
 * @par Toml file example
 * @code
 *      string1 = "hello world"
 *      string2 = """
 *      Roses are red
 *      Violets are blue"""
 *      string3 = """The quick brown fox \
 *      jumps over the lazy dog."""
 *      string4 = 'Tom "Dubs" Preston-Werner'
 *
 *      hex1 = 0xDEADBEEF               # hexadecimal
 *      oct1 = 0o12345670               # octal
 *      bin1 = 0b11010110               # binary
 *      dec1 = 36_414_884               # Decimal

 *      colors = ["red", "blue", "yellow", "green"]
 *      number = [-9,-8,-7,-6,-5,-4,-3,-2,-1,0,1,2,3,4,+5,+6,+7,+8,+9]
 *      fruits = [
 *                  [ ["apple", "banana", "orange"], ["lemon", "peach"] ],
 *                  [ ["pear", "grapes", "strawberry"], ["watermelon", pineapple", "mango"] ],
 *                  "An apple a day keeps the doctor away",
 *                  {apple = "red", banana = "yellow", peach = "pink"}
 *               ]

 *      [animal]
 *      dog.name = "Tina"
 *      dog.age = 4
 *      cat = {name = "Gulu", age = 2}
 *      people = [{name = "Tony", age = 19}, {name = "Jone", age = 21}, {name =
 "Lucy", age = 25}]
 * @endcode
 *
 * @see https://github.com/ToruNiina/toml11/tree/v3.8.1
 */
namespace config_t {

/**
 * @brief Represents a key used in TOML data mappings
 * @details The key can be either an integer or a string. This struct is used to store and distinguish between integer
 * and string keys in TOML data.
 */
struct TomlKey {
    enum class KeyType {
        Integer = 0, ///< The key is an Integer type
        String = 1   ///< The key is a String type
    };
    /**
     * @brief Constructs a TomlKey with an integer key
     * @param[in] index_key The integer key used for mapping data
     */
    explicit TomlKey(int index_key) {
        type_ = KeyType::Integer;
        index_key_ = index_key;
    }
    /**
     * @brief Constructs a TomlKey with a string key
     * @param[in] str_key The string key used for mapping data
     */
    explicit TomlKey(const char* str_key) {
        type_ = KeyType::String;
        str_key_.assign(str_key);
    }
    /**
     * @brief Destruction
     */
    ~TomlKey() = default;

    KeyType type_;        ///< The type of key: integer or string
    int index_key_{};       ///< Store the integer key (if type is integer)
    std::string str_key_; ///< Store the string key (if type is string)
};

/**
 * @brief Creates a vector of TomlKey objects from a single key
 *
 * @tparam T The type of the key, either `const char*` or `int`
 * @param[in] first The first key in the key-value mapping
 *
 * @return A vector containing the single key
 */
template <typename T>
std::vector<TomlKey> MakeArgs(T first) {
    std::vector<TomlKey> vec;
    vec.reserve(6);
    vec.push_back(TomlKey(first));
    return vec;
}

/**
 * @brief Creates a vector of TomlKey objects from multiple keys
 *
 * @tparam T The type of the first key, either `const char*` or `int`
 * @tparam Types Additional types for the other keys
 *
 * @param[in] first The first key in the key-value mapping
 * @param[in] args Additional keys in the mapping
 *
 * @return A vector containing all keys
 */
template <typename T, typename... Types>
std::vector<TomlKey> MakeArgs(T first, Types... args) {
    std::vector<TomlKey> vec = std::move(MakeArgs(args...));
    vec.push_back(TomlKey(first));
    return vec;
}

/**
 * @brief A trusted wrapper around [the untrusted toml library](https://github.com/ToruNiina/toml11/tree/v3.8.1)
 * @details
 * Compared to the [TOML v1.0.0](https://toml.io/en/v1.0.0) standard:
 *  -# We support reading strings and their arrays.
 *  -# We support reading integers (-2^63 to 2^63-1) and their arrays.
 *  -# We do not support reading dates and times or their arrays.
 *  -# We do not support reading floating-point numbers or their arrays.
 *  -# We do not support reading booleans or their arrays.
 */
class TomlConfig {
  public:
    /**
     * @brief Default constructor for the TOML class
     */
    TomlConfig() : ref_untrusted_toml_obj_(0) {};

    /**
     * @brief Destruction
     */
    ~TomlConfig();

  public:
    /**
     * @brief Load a TOML file and initialize the TOML object
     *
     * @param[in] toml_file_path The path to the TOML file to parse
     *
     * @return True if the TOML object was successfully created; false otherwise
     *
     * @par Example
     * @code
     *      bool ok;
     *      TomlConfig toml;
     *      ok = toml.LoadFile(toml, "/root/config");
     *      if (!ok) return false;
     * @endcode
     */
    bool LoadFile(const char* toml_file_path);

    /**
     * @brief Finds an integer value in the TOML file
     *
     * @tparam Types The types of the keys (either `const char*` or `int`)
     *
     * @param[in] args The keys representing the data path to the integer value
     *
     * @return The integer value if success, otherwise return std::nullopt.
     *
     * @par Example
     * @code
     *      bool ok;
     *      TomlConfig toml;
     *      ok = toml.LoadFile("/root/config");
     *      if (!ok) return false;
     *      std::optional<int64_t> value = toml.GetInteger("animal", "dog", "age");
     *      if (!value.has_value()) {
     *          Printf("Failed, error: %s\n", toml.GetLastErrorMsg().c_str());
     *          return false;
     *      }
     *      Printf("Value: %d\n", value);
     * @endcode
     */
    template <typename... Types>
    std::optional<int64_t> GetInteger(Types... args) {
        std::vector<TomlKey> path = std::move(MakeArgs(args...));
        return GetInteger(path);
    };

    /**
     * @brief Finds a string value in the TOML file
     *
     * @tparam Types The types of the keys (either `const char*` or `int`)
     *
     * @param[in] args The keys representing the data path to the string value
     *
     * @return The string value if success, otherwise return std::nullopt.
     *
     * @par Example
     * @code
     *      bool ok;
     *      TomlConfig toml;
     *      ok = toml.LoadFile("/root/config");
     *      if (!ok) return false;
     *      std::optional<std::string>> value = toml.GetString("animal", "dog", "name");
     *      if (!value.has_value()) {
     *          Printf("Failed, error: %s\n", toml.GetLastErrorMsg().c_str());
     *          return false;
     *      }
     *      Printf("Value: %s\n", value.c_str());
     * @endcode
     */
    template <typename... Types>
    std::optional<std::string> GetString(Types... args) {
        std::vector<TomlKey> path = std::move(MakeArgs(args...));
        return GetString(path);
    };

    /**
     * @brief Finds an integer array of values in the TOML file
     *
     * @tparam Types The types of the keys (either `const char*` or `int`)
     *
     * @param[in] args The keys representing the data path to the array of values
     *
     * @return The integer values array if success, otherwise return std::nullopt.
     *
     * @par Example
     * @code
     *      bool ok;
     *      TomlConfig toml;
     *      ok = toml.LoadFile("/root/config");
     *      if (!ok) return false;
     *      std::vector<std::string> values = toml.GetIntegerArray("number");
     *      if (!values.has_value()) {
     *          Printf("Failed, error: %s\n", toml.GetLastErrorMsg().c_str());
     *          return false;
     *      }
     *      Printf("values count: %d\n", values.size());
     * @endcode
     */
    template <typename... Types>
    std::optional<std::vector<int64_t>> GetIntegerArray(Types... args) {
        std::vector<TomlKey> path = std::move(MakeArgs(args...));
        return GetIntegerArray(path);
    };

    /**
     * @brief Finds a string array of values in the TOML file
     *
     * @tparam Types The types of the keys (either `const char*` or `int`)
     *
     * @param[in] args The keys representing the data path to the array of values
     *
     * @return The string values array if success, otherwise return std::nullopt.
     *
     * @par Example
     * @code
     *      bool ok;
     *      TomlConfig toml;
     *      ok = toml.LoadFile("/root/config");
     *      if (!ok) return false;
     *      std::vector<std::string> values = toml.GetStringArray("colors");
     *      if (!values.has_value()) {
     *          Printf("Failed, error: %s\n", toml.GetLastErrorMsg().c_str());
     *          return false;
     *      }
     *      Printf("values count: %d\n", values.size());
     * @endcode
     */
    template <typename... Types>
    std::optional<std::vector<std::string>> GetStringArray(Types... args) {
        std::vector<TomlKey> path = std::move(MakeArgs(args...));
        return GetStringArray(path);
    };

    /**
     * @brief Get the error code when the function returns false
     * @return error code
     *
     * @par Example
     * @code
     *      bool ok;
     *      TomlConfig toml;
     *      ok = toml.LoadFile("/root/config");
     *      if (!ok) {
     *          Printf("error msg: %s\n", toml.GetLastErrorMsg().c_str());
     *          return false;
     *      }
     * @endcode
     * */
    [[nodiscard]] std::string GetLastErrorMsg() const {
        return err_msg_;
    }

  private:
    /**
     * @brief Internal method to get an integer value in the TOML data
     *
     * @param[in] path The vector of keys representing the data path
     *
     * @return The integer value if success, otherwise return std::nullopt.
     */
    std::optional<int64_t> GetInteger(const std::vector<TomlKey>& path);

    /**
     * @brief Internal method to get a string value in the TOML data
     *
     * @param[in] path The vector of keys representing the data path
     *
     * @return The string value if success, otherwise return std::nullopt.
     */
    std::optional<std::string> GetString(const std::vector<TomlKey>& path);

    /**
     * @brief Internal method to get an array of integer values in the TOML data
     *
     * @param[in] path The vector of keys representing the data path
     *
     * @return The integer values array if success, otherwise return std::nullopt.
     */
    std::optional<std::vector<int64_t>> GetIntegerArray(const std::vector<TomlKey>& path);

    /**
     * @brief Internal method to get an array of string values in the TOML data
     *
     * @param[in] path The vector of keys representing the data path
     *
     * @return The string values array if success, otherwise return std::nullopt.
     */
    std::optional<std::vector<std::string>> GetStringArray(const std::vector<TomlKey>& path);

    /**
     * @brief Internal method to get an array value in the TOML data
     *
     * @param[in] ptr_toml The toml object pointer
     * @param[in] path_str The JSON string of search path
     * @param[out] values The result array, in JSON string
     *
     * @return true if success, otherwise return false.
     */
    bool GetArrayValues(uint64_t ptr_toml, const std::string &path_str, std::string &values);

  private:
    uint64_t ref_untrusted_toml_obj_; ///< Reference to the untrusted TOML object
                                      ///< in memory
    std::string err_msg_;             ///< Error code
};

} // namespace config_t
} // namespace ssgx

#endif // SAFEHERON_SGX_TRUSTED_CONFIG_H
