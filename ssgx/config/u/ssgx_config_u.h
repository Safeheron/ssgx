#ifndef SSGXLIB_SSGX_UCONFIG_H
#define SSGXLIB_SSGX_UCONFIG_H

#include "toml.hpp"

namespace ssgx {
namespace config_u {

bool find_toml_node(const char* path, const toml::value* context, toml::value& toml_obj);

}
} // namespace ssgx
#endif // SSGXLIB_SSGX_UCONFIG_H
