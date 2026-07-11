# Using fmt inside an Intel SGX Enclave

> fmt v12.1.0 · header-only · SGX adaptations maintained at [Safeheron/fmt-sgx](https://github.com/Safeheron/fmt-sgx) · installed to `/opt/safeheron/ssgx/include/fmt/` by `external/fmt/apply_patch_and_install.sh`

## How to use

```cpp
#include <fmt/format.h>
// Optional headers:
//   <fmt/ranges.h>   containers / tuple / fmt::join
//   <fmt/args.h>     dynamic_format_arg_store
//   <fmt/compile.h>  FMT_COMPILE compile-time parsing

try {
    auto s = fmt::format(FMT_STRING("user={}, action={}"), uid, action);
    // use s ...
} catch (const std::exception& e) {
    return -1;   // Don't let the exception escape the ecall and kill the enclave.
}
```

**Rules**: wrap format strings in `FMT_STRING(...)` for compile-time checking  ·  `try-catch` at every `fmt::format(...)` call site  ·  use `fmt::format_to_n(buf, n, ...)` for bounded output

## Things to watch out for

| Category | Details |
|---|---|
| ❌ Compile error | `fmt::print` / `println` / `sprintf` / `printf` / `fprintf` / `buffered_file` / `fg` / `bg` / `styled` / `wformat` / chrono formatting; `#include <fmt/{os,ostream,std,xchar,printf,chrono,color}.h>` is accepted but the header is empty — every API inside is compiled out. |
| ⚠️ Silently no-op | `{:L}` (thousands separator) has no effect; `fmt::report_system_error` / `format_system_error` build the message and then discard it. |
| 🔴 Side channel | Beware of side-channel attacks — avoid formatting secret data (keys, passwords, signatures, random seeds) with fmt. Its integer/float paths use data-indexed lookup tables that are readable via cache side channels. |
| 🚫 Compile flag | Do not pass `-fno-exceptions`. Once anything throws, `FMT_THROW` degrades to `abort()` and the enclave terminates with no diagnostic. Do not define `FMT_USE_LOCALE` / `FMT_USE_FLOAT128` yourself (e.g. via `-D`) — the port pins both to 0; overriding them re-enables code paths that do not build inside the Enclave. |
