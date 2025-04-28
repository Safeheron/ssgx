#ifndef SAFEHERON_SGX_LOG_T_H
#define SAFEHERON_SGX_LOG_T_H

#ifdef SAFEHERON_SGX_LOG_U_H
#error "SAFEHERON_SGX_LOG_U_H is already defined. Compilation stopped."
#endif

#include <cstdio>
#include <cstring>
#include <string>

#include "ssgx_log_t_logger.h"

// Internal macro
#define __SSGX__FILENAME__ (strrchr(__FILE__, '/') ? (strrchr(__FILE__, '/') + 1) : __FILE__)

#undef SSGX_LOG
#undef SSGX_LOG_IF
//  @exception RuntimeError Throw exception if logger has not been initialized.
#define SSGX_LOG(LEVEL)                                                                                                \
    ssgx::log_t::internal::LogFinisher() =                                                                             \
        ssgx::log_t::internal::LogMessage(ssgx::log_t::LogLevel::LEVEL, __SSGX__FILENAME__, __LINE__, __FUNCTION__)
#define SSGX_LOG_IF(LEVEL, CONDITION) !(CONDITION) ? (void)0 : SSGX_LOG(LEVEL)

#ifndef SSGX_FUNC_BEGIN
#define SSGX_FUNC_BEGIN (SSGX_LOG(INFO) << "Function Begin!");
#endif

#ifndef SSGX_FUNC_END
#define SSGX_FUNC_END (SSGX_LOG(INFO) << "Function End!");
#endif

#include "ssgx_log_t_logger.h"

#endif // SAFEHERON_SGX_LOG_T_H
