#ifndef SAFEHERON_SGX_TRUSTED_DECIMAL_H
#define SAFEHERON_SGX_TRUSTED_DECIMAL_H

#include <string>

struct mpd_context_t;
struct mpd_t;

namespace ssgx {

/**
 * @brief This module is designed high precision numbers operations in enclave,
 * a Trusted Execution Environment (TEE).
 * @details This module depends on project
 * https://github.com/Safeheron/mpdecimal.git (branch
 * v4.0.0_sgx), which is the Intel SGX version of mpdecimal
 * (https://www.bytereef.org/mpdecimal). mpdeicmal has APIs in both C and C++
 * languages, only C APIs are used in this module. (See
 * https://www.bytereef.org/mpdecimal/doc/libmpdec/index.html)
 *
 * The main functions are as follows:
 * - Addition
 * - Subtraction
 * - Multiplication
 * - Division
 * - Comparison
 * - Rounding
 * - Output as a string
 *
 * @note Big number operations and precision control might cause exceptions,
 *       please refer comments for each functions.
 */
namespace decimal_t {

/**
 * @brief Rounding type for precision control
 *
 */
enum class RoundType {
    RoundUp = 0,        /* round away from 0               */
    RoundDown = 1,      /* round toward 0 (truncate)       */
    RoundCeiling = 2,   /* round toward +infinity          */
    RoundFloor = 3,     /* round toward -infinity          */
    RoundHalfUp = 4,   /* 0.5 is rounded up               */
    RoundHalfDown = 5, /* 0.5 is rounded down             */
    RoundHalfEven = 6, /* 0.5 is rounded to even          */
    Round05Up = 7,      /* round zero or five away from 0  */
    RoundTrunc = 8      /* truncate, but set infinity      */
};

/**
 * @brief Precision control class
 *        The Precision class is used to configure the number of digits and
 *        rounding type of the BigDecimal calculation result.
 *
 */
class Precision {
  public:
    /**
     * @brief Constructor
     *
     * @param[in] round_type The rounding type, one of RoundType value.
     * @param[in] scale > 0, Keep significant digits up to a certain decimal place
     *              = 0, The result will be kept as an integer
     *              < 0, Keep significant digits up to a certain integer place
     *
     * @note Parameter scale must less than SSGX_MAX_PREC(99), otherwise an exception will occur.
     *
     * @code
     * @par Examples
     * @codeotherwise an exception will occur
     *      BigDecimal a("123.456");
     *      BigDecimal b("2.2");
     *      BigDecimal result;
     *      Precision prec1(ROUND_HALF_UP, 2);
     *      Precision prec2(ROUND_HALF_UP, 0);
     *      Precision prec3(ROUND_HALF_UP, -2);
     *
     *      // Use the default precision
     *      result = a.Multiply(b);         // 271.6032
     *
     *      // Round to two decimal places
     *      result = a.Multiply(b, prec1);  // 271.60
     *
     *      // Value kept as an integer
     *      result = a.Multiply(b, prec2);  // 272
     *
     *      // Significant digits up to the hundreds place
     *      result = a.Multiply(b, prec3);  // 300
     *
     * @endcode
     *
     */
    Precision(RoundType round_type, int scale);
    /**
     * @brief Destruction
     */
    ~Precision();

  public:
    friend class BigDecimal;

  private:
    [[nodiscard]] mpd_context_t* Get() const;
    mpd_context_t* ptr_ctx_;
    int scale_;
};

/**
 * @brief High precision big number class
 *        This class implements the basic calculation and comparison operations of high-precision big number library
 * based on mpdecimal C language APIs.
 *
 *        1. The max precision supported is 99 digits, when the maximum precision is exceeded, it will be rounded
 * automatically, the default round type is ROUND_HALF_UP. Using class Precision can customize precision and rounding
 * type for calculation result.
 *
 *        2. According
 * https://github.com/frohoff/jdk8u-dev-jdk/blob/da0da73ab82ed714dc5be94acd2f0d00fbdfe2e9/src/share/classes/java/math/BigDecimal.java#L637
 *           the max exponent supported is 9999999999 (10 nines). it means the value range of Bigdecimal is
 * [-9x10^999999999, 9x10^999999999]. But the actual result depends on the specific hardware performance, an exception
 * will occur if the result exceeds limitations.
 *
 */
class BigDecimal {
  public:
    /**
     * @brief Construct a BigDecimal object and initialized it with "0"
     */
    explicit BigDecimal();

    /**
     * @brief Construct a BigDecimal objet and initialized it with str
     * @param[in] str The string for decimal number
     *
     * @note An exception will occur, if the string
     *       1. is greater than the max value, or
     *       2. is less than the min value, or
     *       3. is an invalid number, or
     *       4. is an unsupported numeric type, such as "infinity" and "Nan".
     *
     */
    explicit BigDecimal(const char* str);

    /**
     * @brief The copy constructor
     *        Construct a BigDecimal objet and copy the value from num
     * @param[in] num The object which value will be copied to this new instance
     */
    BigDecimal(const BigDecimal& num);

    /**
     * @brief The copy assignment operator
     * @param[in] num The object which value will be copied to this new instance
     * @return Return a new BigDecimal object which has the same value as num
     */
    BigDecimal& operator=(const BigDecimal& num);

    /**
     * @brief The move constructor
     *        Construct a BigDecimal object and move the value from num The num object's member pointer ptr_data_ will
     * be set as null
     * @param[in, out] num The object which value will be copied to this new instance, and its member pointer ptr_data_
     * will be set as null
     */
    BigDecimal(BigDecimal&& num) noexcept;

    /**
     * @brief The move and assignment operator
     * @param[in,out] num The object which value will be copied to this new instance, and its member pointer ptr_data_
     * will be set as null
     * @return A new BigDecimal object moved from num.
     */
    BigDecimal& operator=(BigDecimal&& num) noexcept;

    /**
     * @brief Destruction
     */
    virtual ~BigDecimal();

    /**
     * @brief Addition of Numbers
     *        Return a new BigDecimal object which equal to (*this) + num
     * @param[in] num The BigDecimal object which will be added
     * @param[in] prec Used to control the precision of the result
     * @return The addition result object
     *
     * @note 1.By default, the result will be rounded automatically when its number of digits exceeds the default
     * maximum precision, the type of rounding is ROUND_HALF_UP.
     *
     *       2.If a Precision is set, the result will be rounded automatically when its number of digits exceeds
     * Precision.pre, the type of rounding is determined by Precision.round.
     *
     *       3. An exception will occur, if the result exceeds the range of BigDecimal.
     *
     * @code
     * @par Examples
     * @code
     *      BigDecimal a("1.234");
     *      BigDecimal b("2.5");
     *      BigDecimal c = a.Add(b);  // 3.734
     *
     *      Precision prec(20, ROUND_HALF_UP, 2);
     *      BigDecimal d = a.Add(b);  // 3.73
     *
     * @endcode
     */
    [[nodiscard]] BigDecimal Add(const BigDecimal& num) const;
    [[nodiscard]] BigDecimal Add(const BigDecimal& num, const Precision& prec) const;

    /**
     * @brief Subtraction of Numbers.
     *        Return a new BigDecimal object which equal to (*this) - num
     * @param[in] num The BigDecimal object which is the subtrahend
     * @param[in] prec Precision control settings for the result
     * @return The subtraction result object
     *
     * @note 1.By default, the result will be rounded automatically when its number of digits exceeds the default
     * maximum precision, the type of rounding is ROUND_HALF_UP.
     *
     *       2.If a Precision is set, the result will be rounded automatically when its number of digits exceeds
     * Precision.pre, the type of rounding is is determined by Precision.round.
     *
     *       3. An exception will occur, if the result exceeds the range of BigDecimal.
     *
     * @code
     * @par Examples
     * @code
     *      BigDecimal a("10.768");
     *      BigDecimal b("1.2");
     *      BigDecimal c = a.Sub(b);  // 9.568
     *
     *      Precision prec(20, ROUND_HALF_UP, 2);
     *      BigDecimal d = a.Sub(b);  // 9.57
     *
     * @endcode
     */
    [[nodiscard]] BigDecimal Sub(const BigDecimal& num) const;
    [[nodiscard]] BigDecimal Sub(const BigDecimal& num, const Precision& prec) const;

    /**
     * @brief Multiplication of Numbers.
     *        Return a new BigDecimal object which equal to (*this) * num
     * @param[in] num The BigDecimal object which is another multiplier
     * @param[in] prec Precision control settings for the result
     * @return The multiplication result object
     *
     * @note 1.By default, the result will be rounded automatically when its number of digits exceeds the default
     * maximum precision, the type of rounding is ROUND_HALF_UP.
     *
     *       2.If a Precision is set, the result will be rounded automatically when its number of digits exceeds
     * Precision.pre, the type of rounding is determined by Precision.round.
     *
     *       3. An exception will occur, if the result exceeds the range of BigDecimal.
     *
     * @code
     * @par Examples
     * @code
     *      BigDecimal a("1.2345");
     *      BigDecimal b("1.5");
     *      BigDecimal c = a.Multiply(b);  // 1.85175
     *
     *      Precision prec(20, ROUND_HALF_UP, 3);
     *      BigDecimal d = a.Multiply(b);  // 1.852
     *
     * @endcode
     */
    [[nodiscard]] BigDecimal Multiply(const BigDecimal& num) const;
    [[nodiscard]] BigDecimal Multiply(const BigDecimal& num, const Precision& prec) const;

    /**
     * @brief Division of Numbers.
     *        Return a new BigDecimal object which equal to (*this) / num
     * @param[in] num The BigDecimal object which is the divisor
     * @param[in] prec Precision control settings for the result
     * @return The division result object
     *
     * @note 1.By default, the result will be rounded automatically when its number of digits exceeds the default
     * maximum precision, the type of rounding is ROUND_HALF_UP.
     *
     *       2.If a Precision is set, the result will be rounded automatically when its number of digits exceeds
     * Precision.pre, the type of rounding is determined by Precision.round.
     *
     *       3. An exception will occur, if
     *          a) the result exceeds the range of BigDecimal, or
     *          b) 0 is the divisor, or
     *          c) the dividend or divisor is infinity.
     *
     * @code
     * @par Examples
     * @code
     *      BigDecimal a("2.456");
     *      BigDecimal b("2");
     *      BigDecimal c = a.Div(b);  // 1.228
     *
     *      Precision prec(20, ROUND_HALF_UP, 2);
     *      BigDecimal d = a.Div(b);  // 1.23
     *
     * @endcode
     */
    [[nodiscard]] BigDecimal Div(const BigDecimal& num) const;
    [[nodiscard]] BigDecimal Div(const BigDecimal& num, const Precision& prec) const;

    /**
     * @brief Comparison operator: equal to
     * @param[in] num The BigDecimal object to compare
     * @return Return true if (*this) is equal to num, otherwise return false.
     *
     * @note An exception will occur if
     *       1. one of the numbers is Infinity or Nan, or
     *       2. failed to malloc memory
     */
    bool operator==(const BigDecimal& num) const;

    /**
     * @brief Comparison operator: not equal to
     * @param[in] num The BigDecimal object to compare
     * @return Return true if (*this) is not equal to num, otherwise return false.
     *
     * @note An exception will occur if
     *       1. one of the numbers is Infinity or Nan, or
     *       2. failed to malloc memory
     */
    bool operator!=(const BigDecimal& num) const;

    /**
     * @brief Comparison operator: greater than
     * @param[in] num The BigDecimal object to compare
     * @return Return true if (*this) is greater than num, otherwise return false.
     *
     * @note An exception will occur if
     *       1. one of the numbers is Infinity or Nan, or
     *       2. failed to malloc memory
     */
    bool operator>(const BigDecimal& num) const;

    /**
     * @brief Comparison operator: less than
     * @param[in] num The BigDecimal object to compare
     * @return Return true if (*this) is less than num, otherwise return false.
     *
     * @note An exception will occur if
     *       1. one of the numbers is Infinity or Nan, or
     *       2. failed to malloc memory
     */
    bool operator<(const BigDecimal& num) const;

    /**
     * @brief Comparison operator: greater than or equal to
     * @param[in] num The BigDecimal object to compare
     * @return Return true if (*this) is greater than or equal to num, otherwise return false.
     *
     * @note An exception will occur if
     *       1. one of the numbers is Infinity or Nan, or
     *       2. failed to malloc memory
     */
    bool operator>=(const BigDecimal& num) const;

    /**
     * @brief Comparison operator: less than or equal to
     * @param[in] num The BigDecimal object to compare
     * @return Return true if (*this) is less than or equal to num, otherwise return false.
     *
     * @note An exception will occur if
     *       1. one of the numbers is Infinity or Nan, or
     *       2. failed to malloc memory
     */
    bool operator<=(const BigDecimal& num) const;

    /**
     * @brief Set the number of significant decimal places for this BigDecimal
     * @param[in] scale The number of significant digits after the decimal point
     *                  1.scale > 0, Keep significant digits up to a certain decimal place
     *                  2.scale = 0, The result will be kept as an integer
     *                  3.scale < 0, Keep significant digits up to a certain integer place
     * @param[in] round_type Rounding policy, one of RoundType value.
     * @return A new BigDecimal object which significant digits after the decimal point is scale
     *
     * @note An exception will occur if operation is failed.
     *
     * @code
     * @par Examples
     * @code
     *      BigDecimal a("1.23456");
     *      BigDecimal b = a.SetScale(2, ROUND_HALF_UP);     // 1.23;
     *      BigDecimal c = a.SetScale(3, ROUND_HALF_UP);     // 1.235;
     *      BigDecimal d = a.SetScale(10, ROUND_HALF_UP);    // 1.2345600000;
     *
     * @endcode
     *
     */
    [[nodiscard]] BigDecimal SetScale(int scale, RoundType round_type = RoundType::RoundHalfUp) const;

    /**
     * @brief Take the opposite number
     * @return Return a new BigDecimal object which is the opposite of this object
     *
     * @note An exception will occur if operation is failed.
     *
     * @code
     * @par Examples
     * @code
     *      BigDecimal a("2.456");
     *      BigDecimal b = a.Neg();   //b is -2.456
     *
     * @endcode
     */
    [[nodiscard]] BigDecimal Neg() const;

    /**
     * @brief To check a string is a valid expression of BigDecimal or not.
     * @param[in] expr A string
     * @return Return true if expr is a valid expression of BigDecimal, otherwise return fasle.
     *
     * @note An exception will occur if failed to malloc memory.
     *
     * @code
     * @par Examples
     * @code
     *      bool result1 = BigDecimal::IsValidDecimal("2e+4");         // true;
     *      bool result2 = BigDecimal::IsValidDecimal("6(E3)");        // true;
     *      bool result3 = BigDecimal::IsValidDecimal("d7868cdnjc");   // false;
     *      bool result4 = BigDecimal::IsValidDecimal("Nan");          // false;
     *      bool result4 = BigDecimal::IsValidDecimal("infinity");     // false;
     *
     * @endcode
     */
    static bool IsValidDecimal(const char* expr);

    /**
     * @brief Conversion to decimal string in default precision
     * @return A decimal string
     */
    [[nodiscard]] std::string ToStr() const;

    /**
     * @brief Conversion to decimal string is customized precision and rounding
     * policy
     * @param[in] scale The number of significant digits after the decimal point
     *                  1.scale > 0, Keep significant digits up to a certain decimal place
     *                  2.scale = 0, The result will be kept as an integer
     *                  3.scale < 0, Keep significant digits up to a certain integer place
     * @param[in] round_type Rounding policy, one of RoundType value.
     * @return A decimal string
     *
     * @code
     * @par Examples
     * @code
     *      BigDecimal a("2.456");
     *      std::string str1 = a.ToStr(1, ROUND_HALF_UP);       // 2.5
     *      std::string str2 = a.ToStr(1, ROUND_HALF_DOWN);     // 2.4
     *
     * @endcode
     */
    [[nodiscard]] std::string ToStr(int scale, RoundType round_type) const;

  private:
    int Compare(const BigDecimal& num) const;
    static void SetDefaultContext(mpd_context_t* ptr_ctx);

  private:
    mpd_t* ptr_data_;
};

} // namespace decimal_t
} // namespace ssgx

#endif // SAFEHERON_SGX_TRUSTED_DECIMAL_H
