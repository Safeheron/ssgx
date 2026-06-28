#ifndef SSGXLIB_SSGX_UCONFIG_H
#define SSGXLIB_SSGX_UCONFIG_H

#include "toml.hpp"

namespace ssgx {
namespace config_u {

bool find_toml_node(const char* path, const toml::ordered_value* context, toml::ordered_value& toml_obj);

int zero_file_content(const char* file_path);

}
} // namespace ssgx
#endif // SSGXLIB_SSGX_UCONFIG_H
