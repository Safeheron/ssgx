#include "ssgx_decimal_t.h"

#include <assert.h>
#include <cstring>
#include <stdexcept>

#include "custom_assert.h"
#include "mpdecimal/mpdecimal.h"

// The max precision supported
#define SSGX_MAX_PREC 99

// Refer
// https://github.com/frohoff/jdk8u-dev-jdk/blob/da0da73ab82ed714dc5be94acd2f0d00fbdfe2e9/src/share/classes/java/math/BigDecimal.java#L637
// To limit the max exponent to 9999999999 (10 nines)
#define SSGX_MAX_EXP 9999999999L
#define SSGX_MIN_EXP -9999999999L

namespace ssgx {
namespace decimal_t {

/**
 * @brief Convert RoundType to mpdecimal round type
 *
 * @param round_type A RoundType value
 * @return int The converted mpdecimal round type value, return -1 if round_type is an invalid type
 */
static int MapRoundType(RoundType round_type) {
    switch (round_type) {
    case RoundType::RoundUp:
        return MPD_ROUND_UP;
    case RoundType::RoundDown:
        return MPD_ROUND_DOWN;
    case RoundType::RoundCeiling:
        return MPD_ROUND_CEILING;
    case RoundType::RoundFloor:
        return MPD_ROUND_FLOOR;
    case RoundType::RoundHalfUp:
        return MPD_ROUND_HALF_UP;
    case RoundType::RoundHalfDown:
        return MPD_ROUND_HALF_DOWN;
    case RoundType::RoundHalfEven:
        return MPD_ROUND_HALF_EVEN;
    case RoundType::Round05Up:
        return MPD_ROUND_05UP;
    case RoundType::RoundTrunc:
        return MPD_ROUND_TRUNC;
    default:
        return -1;
    }
}

/**
 * @brief Check mpdecimal context's status, in this way we can identiy
 *        current operation is successful or not.
 *
 * @param[in] ctx Current mpdecimal content object
 * @param[in] ignore_rounding No error message for MPD_Inexact and MPD_Rounded
 * @param[out] error Error messages
 * @return true No any concern exceptions occurred
 * @return false Has one or more concern exceptions
 */
static bool CheckContextStatus(const mpd_context_t* ctx, bool ignore_rounding, std::string& error) {
    uint32_t status = mpd_getstatus(ctx);
    error.clear();

    if (status & MPD_Clamped) {
        error.append("MPD_Clamped occurred! The result's exponent exceeds the valid rangen!\n");
    }
    if (status & MPD_Conversion_syntax) {
        error.append("MPD_Conversion_syntax occurred! This may be because an invalid string was used.\n");
    }
    if (status & MPD_Division_by_zero) {
        error.append("MPD_Division_by_zero occurred! 0 cannot be used as a divisor.\n");
    }
    if (status & MPD_Division_impossible) {
        // In ssgx, this will never happen
        error.append("MPD_Division_impossible occurred! Division operation cannot be performed due to the lack of "
                     "sufficient precision in the current context.\n");
    }
    if (status & MPD_Division_undefined) {
        // This will only happen when 0/0
        error.append("MPD_Division_undefined occurred! An undefined division operation is attempted.\n");
    }
    if (status & MPD_Fpu_error) {
        error.append("MPD_Fpu_error occurred! A floating-point error encountered in mathematical computations.\n");
    }
    if (status & MPD_Inexact) {
        // Show error message if ignore_rounding is false
        if (!ignore_rounding)
            error.append(
                "MPD_Inexact occurred! The result is inexact because it cannot be presented in current precision.\n");
    }
    if (status & MPD_Invalid_context) {
        error.append("MPD_Invalid_context occurred! The precision must be greater than 0 and less than 99)!\n");
    }
    if (status & MPD_Invalid_operation) {
        error.append("MPD_Invalid_operation occurred! An invalid operation!\n");
    }
    if (status & MPD_Malloc_error) {
        error.append("MPD_Malloc_error occurred! Failed to allocate memory for objects.\n");
    }
    if (status & MPD_Not_implemented) {
        error.append("MPD_Not_implemented occurred! This function is not implemented yet.\n");
    }
    if (status & MPD_Overflow) {
        error.append("MPD_Overflow occurred! The result is too large to be represented as a normal number within the "
                     "current context.\n");
    }
    if (status & MPD_Rounded) {
        // Show error message if ignore_rounding is false
        if (!ignore_rounding)
            error.append("MPD_Rounded occurred! The result is rounded because the number of digits exceeds current.\n");
    }
    if (status & MPD_Subnormal) {
        error.append("MPD_Subnormal occurred! The result is a subnormal number.\n");
    }
    if (status & MPD_Underflow) {
        error.append("MPD_Underflow occurred! The result is too small to be represented as a normal number within the "
                     "current context.\n");
    }

    return error.empty();
}

Precision::Precision(RoundType round_type, int scale) : ptr_ctx_(nullptr), scale_(0) {
    std::string error_msg;

    if (scale >= SSGX_MAX_PREC) {
        throw ssgx::exception_t::LocatedException(__FILE__, __LINE__, __FUNCTION__, -1,
                                                  "Invalid parameter! scale must less than 99.");
    }

    if (!(ptr_ctx_ = new mpd_context_t)) {
        throw ssgx::exception_t::LocatedException(__FILE__, __LINE__, __FUNCTION__, -2,
                                                  "A malloc error! Failed to new mpd_context_t.");
    }

    mpd_init(ptr_ctx_, SSGX_MAX_PREC);
    if (!CheckContextStatus(ptr_ctx_, false, error_msg)) {
        delete ptr_ctx_;
        ptr_ctx_ = nullptr;
        throw ssgx::exception_t::LocatedException(__FILE__, __LINE__, __FUNCTION__, -3, error_msg.c_str());
    }

    if (!mpd_qsetround(ptr_ctx_, MapRoundType(round_type))) {
        delete ptr_ctx_;
        ptr_ctx_ = nullptr;
        throw ssgx::exception_t::LocatedException(__FILE__, __LINE__, __FUNCTION__, -4,
                                                  "Parameter round_type is invalid.");
    }

    scale_ = scale;
}

Precision::~Precision() {
    if (ptr_ctx_) {
        delete ptr_ctx_;
        ptr_ctx_ = nullptr;
    }
}

mpd_context_t* Precision::Get() const {
    return ptr_ctx_;
}

BigDecimal::BigDecimal() : BigDecimal("0") {
}

BigDecimal::BigDecimal(const char* str) {
    mpd_context_t ctx;
    std::string error_msg;

    // Validate str, it must not be empty,
    // and its max length must be less than SSGX_MAX_PREC + 10.
    if (!str || strnlen(str, 1) == 0 || strlen(str) > SSGX_MAX_PREC + 10) {
        throw ssgx::exception_t::LocatedException(
            __FILE__, __LINE__, __FUNCTION__, -1,
            "Invalid parameter! str must not be empty, or its length exceeds the max.");
    }

    SetDefaultContext(&ctx);
    if (!(ptr_data_ = mpd_new(&ctx))) {
        throw ssgx::exception_t::LocatedException(__FILE__, __LINE__, __FUNCTION__, -2,
                                                  "A malloc error! Failed to call mpd_new().");
    }

    mpd_set_string(ptr_data_, str, &ctx);

    // Consider the result is invalid if meet one of the following conditions
    // 1. Any exceptions
    // 2. Result is infinite
    // 3. Result is NaN
    if (!CheckContextStatus(&ctx, false, error_msg)) {
        mpd_del(ptr_data_);
        ptr_data_ = nullptr;
        throw ssgx::exception_t::LocatedException(__FILE__, __LINE__, __FUNCTION__, -3, error_msg.c_str());
    }
    if (mpd_isinfinite(ptr_data_) || mpd_isnan(ptr_data_)) {
        mpd_del(ptr_data_);
        ptr_data_ = nullptr;
        throw ssgx::exception_t::LocatedException(__FILE__, __LINE__, __FUNCTION__, -4,
                                                  "Paramter str is an invalid number.");
    }
}

BigDecimal::BigDecimal(const BigDecimal& num) {
    mpd_context_t ctx;
    std::string error_msg;

    ASSERT_THROW(num.ptr_data_);

    SetDefaultContext(&ctx);
    if (!(ptr_data_ = mpd_new(&ctx))) {
        throw ssgx::exception_t::LocatedException(__FILE__, __LINE__, __FUNCTION__, -1,
                                                  "A malloc error! Failed to call mpd_new().");
    }

    mpd_copy(ptr_data_, num.ptr_data_, &ctx);
    if (!CheckContextStatus(&ctx, false, error_msg)) {
        mpd_del(ptr_data_);
        ptr_data_ = nullptr;
        throw ssgx::exception_t::LocatedException(__FILE__, __LINE__, __FUNCTION__, -2, error_msg.c_str());
    }
}

BigDecimal::BigDecimal(BigDecimal&& num) noexcept {
    ptr_data_ = num.ptr_data_;
    num.ptr_data_ = nullptr;
}

BigDecimal& BigDecimal::operator=(const BigDecimal& num) {
    mpd_context_t ctx;
    std::string error_msg;

    if (this == &num)
        return *this;

    ASSERT_THROW(num.ptr_data_);

    SetDefaultContext(&ctx);
    if (ptr_data_) {
        delete ptr_data_;
        ptr_data_ = nullptr;
    }
    if (!(ptr_data_ = mpd_new(&ctx))) {
        throw ssgx::exception_t::LocatedException(__FILE__, __LINE__, __FUNCTION__, -1,
                                                  "A malloc error! Failed to call mpd_new().");
    }

    mpd_copy(ptr_data_, num.ptr_data_, &ctx);
    if (!CheckContextStatus(&ctx, false, error_msg)) {
        mpd_del(ptr_data_);
        ptr_data_ = nullptr;
        throw ssgx::exception_t::LocatedException(__FILE__, __LINE__, __FUNCTION__, -2, error_msg.c_str());
    }

    return *this;
}

BigDecimal& BigDecimal::operator=(BigDecimal&& num) noexcept {
    if (this == &num)
        return *this;

    if (ptr_data_) {
        mpd_del(ptr_data_);
        ptr_data_ = nullptr;
    }

    ptr_data_ = num.ptr_data_;
    num.ptr_data_ = nullptr;

    return *this;
}

BigDecimal::~BigDecimal() {
    if (ptr_data_) {
        mpd_del(ptr_data_);
        ptr_data_ = nullptr;
    }
}

BigDecimal BigDecimal::Add(const BigDecimal& num) const {
    BigDecimal result;
    mpd_context_t ctx;
    std::string error_msg;

    ASSERT_THROW(num.ptr_data_);
    ASSERT_THROW(ptr_data_);

    SetDefaultContext(&ctx);
    mpd_add(result.ptr_data_, ptr_data_, num.ptr_data_, &ctx);
    if (!CheckContextStatus(&ctx, false, error_msg)) {
        throw ssgx::exception_t::LocatedException(__FILE__, __LINE__, __FUNCTION__, -1, error_msg.c_str());
    }
    return result;
}

BigDecimal BigDecimal::Add(const BigDecimal& num, const Precision& prec) const {
    BigDecimal result;
    std::string error_msg;
    mpd_context_t ctx = *prec.Get();

    ASSERT_THROW(num.ptr_data_);
    ASSERT_THROW(ptr_data_);

    mpd_add(result.ptr_data_, ptr_data_, num.ptr_data_, &ctx);
    if (!CheckContextStatus(&ctx, true, error_msg)) {
        throw ssgx::exception_t::LocatedException(__FILE__, __LINE__, __FUNCTION__, -1, error_msg.c_str());
    }

    return result.SetScale(prec.scale_, (RoundType)ctx.round);
}

BigDecimal BigDecimal::Sub(const BigDecimal& num) const {
    BigDecimal result;
    mpd_context_t ctx;
    std::string error_msg;

    ASSERT_THROW(num.ptr_data_);
    ASSERT_THROW(ptr_data_);

    SetDefaultContext(&ctx);
    mpd_sub(result.ptr_data_, ptr_data_, num.ptr_data_, &ctx);
    if (!CheckContextStatus(&ctx, false, error_msg)) {
        throw ssgx::exception_t::LocatedException(__FILE__, __LINE__, __FUNCTION__, -1, error_msg.c_str());
    }

    return result;
}

BigDecimal BigDecimal::Sub(const BigDecimal& num, const Precision& prec) const {
    BigDecimal result;
    std::string error_msg;
    mpd_context_t ctx = *prec.Get();

    ASSERT_THROW(num.ptr_data_);
    ASSERT_THROW(ptr_data_);

    mpd_sub(result.ptr_data_, ptr_data_, num.ptr_data_, &ctx);
    if (!CheckContextStatus(&ctx, true, error_msg)) {
        throw ssgx::exception_t::LocatedException(__FILE__, __LINE__, __FUNCTION__, -1, error_msg.c_str());
    }

    return result.SetScale(prec.scale_, (RoundType)ctx.round);
}

BigDecimal BigDecimal::Multiply(const BigDecimal& num) const {
    BigDecimal result;
    mpd_context_t ctx;
    std::string error_msg;

    ASSERT_THROW(num.ptr_data_);
    ASSERT_THROW(ptr_data_);

    SetDefaultContext(&ctx);
    mpd_mul(result.ptr_data_, ptr_data_, num.ptr_data_, &ctx);
    if (!CheckContextStatus(&ctx, false, error_msg)) {
        throw ssgx::exception_t::LocatedException(__FILE__, __LINE__, __FUNCTION__, -1, error_msg.c_str());
    }

    return result;
}

BigDecimal BigDecimal::Multiply(const BigDecimal& num, const Precision& prec) const {
    BigDecimal result;
    std::string error_msg;
    mpd_context_t ctx = *prec.Get();

    ASSERT_THROW(num.ptr_data_);
    ASSERT_THROW(ptr_data_);

    mpd_mul(result.ptr_data_, ptr_data_, num.ptr_data_, &ctx);
    if (!CheckContextStatus(&ctx, true, error_msg)) {
        throw ssgx::exception_t::LocatedException(__FILE__, __LINE__, __FUNCTION__, -1, error_msg.c_str());
    }

    return result.SetScale(prec.scale_, (RoundType)ctx.round);
}

BigDecimal BigDecimal::Div(const BigDecimal& num) const {
    BigDecimal result;
    mpd_context_t ctx;
    std::string error_msg;

    ASSERT_THROW(num.ptr_data_);
    ASSERT_THROW(ptr_data_);

    SetDefaultContext(&ctx);
    mpd_div(result.ptr_data_, ptr_data_, num.ptr_data_, &ctx);
    if (!CheckContextStatus(&ctx, false, error_msg)) {
        throw ssgx::exception_t::LocatedException(__FILE__, __LINE__, __FUNCTION__, -1, error_msg.c_str());
    }

    return result;
}

BigDecimal BigDecimal::Div(const BigDecimal& num, const Precision& prec) const {
    BigDecimal result;
    std::string error_msg;
    mpd_context_t ctx = *prec.Get();

    ASSERT_THROW(num.ptr_data_);
    ASSERT_THROW(ptr_data_);

    mpd_div(result.ptr_data_, ptr_data_, num.ptr_data_, &ctx);
    if (!CheckContextStatus(&ctx, true, error_msg)) {
        throw ssgx::exception_t::LocatedException(__FILE__, __LINE__, __FUNCTION__, -1, error_msg.c_str());
    }

    return result.SetScale(prec.scale_, (RoundType)ctx.round);
}

bool BigDecimal::operator==(const BigDecimal& num) const {
    return Compare(num) == 0;
}

bool BigDecimal::operator!=(const BigDecimal& num) const {
    return Compare(num) != 0;
}

bool BigDecimal::operator>(const BigDecimal& num) const {
    return Compare(num) == 1;
}

bool BigDecimal::operator<(const BigDecimal& num) const {
    return Compare(num) == -1;
}

bool BigDecimal::operator>=(const BigDecimal& num) const {
    return Compare(num) == 1 || Compare(num) == 0;
}

bool BigDecimal::operator<=(const BigDecimal& num) const {
    return Compare(num) == -1 || Compare(num) == 0;
}

BigDecimal BigDecimal::SetScale(int scale, RoundType round_type) const {
    BigDecimal result;
    mpd_t* scale_num = nullptr;
    std::string scale_str = "1e";
    mpd_context_t ctx;
    std::string error_msg;

    ASSERT_THROW(ptr_data_);

    if (scale >= SSGX_MAX_PREC) {
        throw ssgx::exception_t::LocatedException(__FILE__, __LINE__, __FUNCTION__, -1,
                                                  "Invalid parameter! scale must less than 99.");
    }

    SetDefaultContext(&ctx);
    if (!(scale_num = mpd_new(&ctx))) {
        throw ssgx::exception_t::LocatedException(__FILE__, __LINE__, __FUNCTION__, -2,
                                                  "An malloc error! Failed to call mpd_new().");
    }
    scale_str += std::to_string(-scale);
    mpd_set_string(scale_num, scale_str.c_str(), &ctx);
    if (!CheckContextStatus(&ctx, false, error_msg)) {
        mpd_del(scale_num);
        scale_num = nullptr;
        throw ssgx::exception_t::LocatedException(__FILE__, __LINE__, __FUNCTION__, -3, error_msg.c_str());
    }

    SetDefaultContext(&ctx);
    if (!mpd_qsetround(&ctx, MapRoundType(round_type))) {
        throw ssgx::exception_t::LocatedException(__FILE__, __LINE__, __FUNCTION__, -4,
                                                  "Parameter round_type is invalid.");
    }
    mpd_quantize(result.ptr_data_, ptr_data_, scale_num, &ctx);
    if (!CheckContextStatus(&ctx, true, error_msg)) {
        mpd_del(scale_num);
        scale_num = nullptr;
        throw ssgx::exception_t::LocatedException(__FILE__, __LINE__, __FUNCTION__, -5, error_msg.c_str());
    }

    mpd_del(scale_num);
    scale_num = nullptr;

    return result;
}

BigDecimal BigDecimal::Neg() const {
    BigDecimal result;
    BigDecimal zero;
    mpd_context_t ctx;
    std::string error_msg;

    ASSERT_THROW(ptr_data_);

    SetDefaultContext(&ctx);
    mpd_sub(result.ptr_data_, zero.ptr_data_, ptr_data_, &ctx);
    if (!CheckContextStatus(&ctx, true, error_msg)) {
        throw ssgx::exception_t::LocatedException(__FILE__, __LINE__, __FUNCTION__, -1, error_msg.c_str());
    }

    return result;
}

bool BigDecimal::IsValidDecimal(const char* expr) {
    bool valid = false;
    mpd_context_t ctx;
    mpd_t* ptr = nullptr;
    std::string error_msg;

    SetDefaultContext(&ctx);
    if (!(ptr = mpd_new(&ctx))) {
        throw ssgx::exception_t::LocatedException(__FILE__, __LINE__, __FUNCTION__, -1,
                                                  "An malloc error! Failed to call mpd_new().");
    }

    // Rounding, overflow etc. exceptions can take place.
    // If the string is not valid, the MPD_Conversion_syntax condition
    // is added to status and result is set to NaN.
    mpd_set_string(ptr, expr, &ctx);

    // Set valid = true only when meet all the following conditions
    // 1. No exceptions
    // 2. Not infinite
    // 3. Not NaN
    valid = (CheckContextStatus(&ctx, false, error_msg)) && !mpd_isinfinite(ptr) && !mpd_isnan(ptr);

    mpd_del(ptr);
    ptr = nullptr;

    return valid;
}

std::string BigDecimal::ToStr() const {
    std::string result;
    char* sci_str = nullptr;

    ASSERT_THROW(ptr_data_);

    if (!(sci_str = mpd_to_sci(ptr_data_, 1))) {
        throw ssgx::exception_t::LocatedException(__FILE__, __LINE__, __FUNCTION__, -1,
                                                  "Failed to convert decimal to string.");
    }

    result.assign(sci_str, strlen(sci_str));
    mpd_free(sci_str);
    sci_str = nullptr;
    return result;
}

std::string BigDecimal::ToStr(int scale, RoundType round_type) const {
    return SetScale(scale, round_type).ToStr();
}

int BigDecimal::Compare(const BigDecimal& num) const {
    int ret = 0;
    mpd_context_t ctx;

    ASSERT_THROW(num.ptr_data_);
    ASSERT_THROW(ptr_data_);

    SetDefaultContext(&ctx);

    // Return -1 if ptr_data_ is less than num.ptr_data_;
    // Return 0 if ptr_data_ is equal to num.ptr_data_;
    // Return 1 if ptr_data_ is greater than num.ptr_data_.
    //
    // The MPD_Invalid_operation condition is added to status if at least
    // one of the operands is a signaling NaN. In this case, result is set
    // to NaN and INT_MAX is returned.
    ret = mpd_cmp(ptr_data_, num.ptr_data_, &ctx);

    // Show an exception if is an invalid operation.
    if (ret == INT_MAX) {
        throw ssgx::exception_t::LocatedException(
            __FILE__, __LINE__, __FUNCTION__, -1,
            "Invalid operation! At least one of the operands is a signaling NaN.");
    }

    return ret;
}

void BigDecimal::SetDefaultContext(mpd_context_t* ptr_ctx) {
    // In default, use the max context,
    // and set prec, emax, emin and round type
    // to our customerized values
    if (ptr_ctx) {
        mpd_maxcontext(ptr_ctx);
        ptr_ctx->prec = SSGX_MAX_PREC;
        ptr_ctx->emax = SSGX_MAX_EXP;
        ptr_ctx->emin = SSGX_MIN_EXP;
        ptr_ctx->round = MPD_ROUND_HALF_UP;
    }
}

} // namespace decimal_t
} // namespace ssgx