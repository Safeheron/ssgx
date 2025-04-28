#include "ssgx_decimal_t.h"
#include "ssgx_exception_t.h"
#include "ssgx_testframework_t.h"

using namespace ssgx::decimal_t;

TEST(DecimalTestSuite, TestValidNum) {
    ASSERT_FALSE(BigDecimal::IsValidDecimal(""));

    ASSERT_FALSE(BigDecimal::IsValidDecimal("3821*32"));

    ASSERT_FALSE(BigDecimal::IsValidDecimal("#382132384"));

    ASSERT_FALSE(BigDecimal::IsValidDecimal("d7868cdnjc"));

    ASSERT_FALSE(BigDecimal::IsValidDecimal("(34141)"));

    ASSERT_FALSE(BigDecimal::IsValidDecimal("Pi"));

    ASSERT_TRUE(BigDecimal::IsValidDecimal(".4243284728"));

    ASSERT_FALSE(BigDecimal::IsValidDecimal("3821.0."));

    ASSERT_FALSE(BigDecimal::IsValidDecimal("+-2299"));

    ASSERT_TRUE(BigDecimal::IsValidDecimal("-.666666"));

    ASSERT_TRUE(BigDecimal::IsValidDecimal("893248329483924823430948320.480238408499"));

    ASSERT_FALSE(BigDecimal::IsValidDecimal(
        "893248329483924823430948320."
        "4802384084996867689867686868768768686786786867676868686786378462764826347826487326482647823"));

    ASSERT_FALSE(BigDecimal::IsValidDecimal("+3242-45352"));

    ASSERT_FALSE(BigDecimal::IsValidDecimal(".+44"));

    ASSERT_FALSE(BigDecimal::IsValidDecimal("+Infinity"));

    ASSERT_FALSE(BigDecimal::IsValidDecimal("NaN"));

    ASSERT_TRUE(BigDecimal::IsValidDecimal("2e+4"));

    ASSERT_TRUE(BigDecimal::IsValidDecimal("2E-55"));

    ASSERT_FALSE(BigDecimal::IsValidDecimal("2E-55+1"));

    ASSERT_TRUE(BigDecimal::IsValidDecimal("-.0000006328E+77777"));

    ASSERT_TRUE(BigDecimal::IsValidDecimal("3E333"));

    ASSERT_FALSE(BigDecimal::IsValidDecimal("5EE666"));

    ASSERT_FALSE(BigDecimal::IsValidDecimal("6E6E4"));

    ASSERT_FALSE(BigDecimal::IsValidDecimal("6(E3)"));

    ASSERT_TRUE(BigDecimal::IsValidDecimal("4E+9999999999"));

    ASSERT_TRUE(BigDecimal::IsValidDecimal("0.04E+10000000001"));

    ASSERT_FALSE(BigDecimal::IsValidDecimal("4E+10000000000"));

    ASSERT_FALSE(BigDecimal::IsValidDecimal("44E+9999999999"));

    ASSERT_TRUE(BigDecimal::IsValidDecimal("4E-9999999999"));

    ASSERT_FALSE(BigDecimal::IsValidDecimal("4E-10000000000"));
}

TEST(DecimalTestSuite, TestComparison) {
    BigDecimal a("1");
    BigDecimal b("1.000000");
    std::vector<bool> comparison_result = {a<b, a> b, a == b, a <= b, a >= b};
    std::vector<bool> comparison_result_expected = {false, false, true, true, true};
    ASSERT_EQ(comparison_result, comparison_result_expected);

    a = BigDecimal("123456789");
    b = BigDecimal("123456789.87654321");
    comparison_result = {a<b, a> b, a == b, a <= b, a >= b};
    comparison_result_expected = {true, false, false, true, false};
    ASSERT_EQ(comparison_result, comparison_result_expected);

    a = BigDecimal("123456789");
    b = BigDecimal("12345678.987654321");
    comparison_result = {a<b, a> b, a == b, a <= b, a >= b};
    comparison_result_expected = {false, true, false, false, true};
    ASSERT_EQ(comparison_result, comparison_result_expected);

    a = BigDecimal("-43242");
    b = BigDecimal("3434");
    comparison_result = {a<b, a> b, a == b, a <= b, a >= b};
    comparison_result_expected = {true, false, false, true, false};
    ASSERT_EQ(comparison_result, comparison_result_expected);

    a = BigDecimal("-88888888888");
    b = BigDecimal("-77777777777");
    comparison_result = {a<b, a> b, a == b, a <= b, a >= b};
    comparison_result_expected = {true, false, false, true, false};
    ASSERT_EQ(comparison_result, comparison_result_expected);

    a = BigDecimal("324373424342.34243243242342300");
    b = BigDecimal("324373424342.342432432423423000000000001");
    comparison_result = {a<b, a> b, a == b, a <= b, a >= b};
    comparison_result_expected = {true, false, false, true, false};
    ASSERT_EQ(comparison_result, comparison_result_expected);

    a = BigDecimal("999999999999999999999999999999999999999");
    b = BigDecimal("1000000000000000000000000000000000000000");
    comparison_result = {a<b, a> b, a == b, a <= b, a >= b};
    comparison_result_expected = {true, false, false, true, false};
    ASSERT_EQ(comparison_result, comparison_result_expected);

    a = BigDecimal("-99999999999999999999999999999999999999999999999999999");
    b = BigDecimal("+99999999999999999999999999999999999999999999999999999");
    comparison_result = {a<b, a> b, a == b, a <= b, a >= b};
    comparison_result_expected = {true, false, false, true, false};
    ASSERT_EQ(comparison_result, comparison_result_expected);

    a = BigDecimal("3e999");
    b = BigDecimal("9e888888");
    comparison_result = {a<b, a> b, a == b, a <= b, a >= b};
    comparison_result_expected = {true, false, false, true, false};
    ASSERT_EQ(comparison_result, comparison_result_expected);

    a = BigDecimal("9e98").Sub(BigDecimal("1"));
    b = BigDecimal("9e98").Add(BigDecimal("1"));
    comparison_result = {a<b, a> b, a == b, a <= b, a >= b};
    comparison_result_expected = {true, false, false, true, false};
    ASSERT_EQ(comparison_result, comparison_result_expected);

    a = BigDecimal("+0");
    b = BigDecimal("-0");
    comparison_result = {a<b, a> b, a == b, a <= b, a >= b};
    comparison_result_expected = {false, false, true, true, true};
    ASSERT_EQ(comparison_result, comparison_result_expected);
}

TEST(DecimalTestSuite, TestAdd) {
    BigDecimal a("1.1");
    BigDecimal b("3.14");
    BigDecimal result = a.Add(b);
    ASSERT_EQ(result, BigDecimal("4.24"));
    Precision prec1(RoundType::RoundHalfEven, 1);
    result = a.Add(b, prec1);
    ASSERT_EQ(result, BigDecimal("4.2"));

    a = BigDecimal("1234567890.111111111111");
    b = BigDecimal("23455.333333333");
    result = a.Add(b);
    ASSERT_EQ(result, BigDecimal("1234591345.444444444111"));
    Precision prec2(RoundType::RoundDown, 4);
    result = a.Add(b, prec2);
    ASSERT_EQ(result, BigDecimal("1234591345.4444"));

    a = BigDecimal("100000000000000000000000000000000000000001.99");
    b = BigDecimal("0.333");
    result = a.Add(b);
    ASSERT_EQ(result, BigDecimal("100000000000000000000000000000000000000002.323"));
    Precision prec3(RoundType::RoundHalfEven, -2);
    result = a.Add(b, prec3);
    ASSERT_EQ(result, BigDecimal("1e41"));

    a = BigDecimal("-999999999999999999999999999999999999999999999");
    b = BigDecimal("1000000000000000000000000000000000000000000000");
    result = a.Add(b);
    ASSERT_EQ(result, BigDecimal("1"));

    a = BigDecimal("-0.0000030000000000000000000033000000000000009");
    b = BigDecimal("-3.0000000000000000000000000000000300000000001");
    result = a.Add(b);
    ASSERT_EQ(result, BigDecimal("-3.000003000000000000000000003300030000000001"));
    Precision prec4(RoundType::RoundUp, 38);
    result = a.Add(b, prec4);
    ASSERT_EQ(result, BigDecimal("-3.00000300000000000000000000330003000001"));

    a = BigDecimal("-0.999999999999999999999999999999999999999999999");
    b = BigDecimal("1");
    result = a.Add(b);
    ASSERT_EQ(result, BigDecimal("1E-45"));
    Precision prec5(RoundType::RoundFloor, 30);
    result = a.Add(b, prec5);
    ASSERT_EQ(result, BigDecimal("0"));

    a = BigDecimal("+9E+999999");
    b = BigDecimal("-9E+999998");
    result = a.Add(b);
    ASSERT_EQ(result, BigDecimal("8.1E+999999"));
}

TEST(DecimalTestSuite, TestSub) {
    BigDecimal a("1234567890.111111111111");
    BigDecimal b("23455.333333333");
    BigDecimal result = a.Sub(b);
    ASSERT_EQ(result, BigDecimal("1234544434.777777778111"));
    Precision prec1(RoundType::RoundCeiling, 4);
    result = a.Sub(b, prec1);
    ASSERT_EQ(result, BigDecimal("1234544434.7778"));
    Precision prec2(RoundType::RoundHalfEven, -2);
    result = a.Sub(b, prec2);
    ASSERT_EQ(result, BigDecimal("1234544400"));

    a = BigDecimal("0.99999999999999999999999999999");
    b = BigDecimal("1");
    result = a.Sub(b);
    ASSERT_EQ(result, BigDecimal("-1E-29"));

    a = BigDecimal("9999999999999999999999999999999999999999.99");
    b = BigDecimal("0.99");
    result = a.Sub(b);
    ASSERT_EQ(result, BigDecimal("9999999999999999999999999999999999999999"));
    Precision prec3(RoundType::RoundHalfEven, -8);
    result = a.Sub(b, prec3);
    ASSERT_EQ(result, BigDecimal("1e40"));
}

TEST(DecimalTestSuite, TestMultiply) {
    BigDecimal a("32342.23131");
    BigDecimal b("-0.2313");
    BigDecimal result = a.Multiply(b);
    ASSERT_EQ(result, BigDecimal("-7480.758102003"));
    Precision prec1(RoundType::RoundHalfEven, 6);
    result = a.Multiply(b, prec1);
    ASSERT_EQ(result, BigDecimal("-7480.758102"));

    a = BigDecimal("-0.3824791749");
    b = BigDecimal("-0.74923749");
    result = a.Multiply(b);
    ASSERT_EQ(result, BigDecimal("0.286567736979347001"));
    Precision prec2(RoundType::RoundHalfEven, 4);
    result = a.Multiply(b, prec2);
    ASSERT_EQ(result, BigDecimal("0.2866"));

    a = BigDecimal("8932483294839248234309483204802.38408499");
    b = BigDecimal("123");
    result = a.Multiply(b);
    ASSERT_EQ(result, BigDecimal("1098695445265227532820066434190693.24245377"));
    Precision prec3(RoundType::RoundFloor, 6);
    result = a.Multiply(b, prec3);
    ASSERT_EQ(result, BigDecimal("1098695445265227532820066434190693.242453"));

    a = BigDecimal("8932483294839248234309483204802");
    b = BigDecimal("10");
    Precision prec4(RoundType::RoundHalfEven, -6);
    result = a.Multiply(b, prec4);
    ASSERT_EQ(result, BigDecimal("89324832948392482343094832000000"));

    a = BigDecimal("-5E+999999");
    b = BigDecimal("+5E-999999");
    result = a.Multiply(b);
    ASSERT_EQ(result, BigDecimal("-25"));

    a = BigDecimal("+9E+999999");
    b = BigDecimal("0");
    result = a.Multiply(b);
    ASSERT_EQ(result, BigDecimal("0"));
}

TEST(DecimalTestSuite, TestDiv) {
    BigDecimal a("1");
    BigDecimal b("1024");
    BigDecimal result = a.Div(b);
    ASSERT_EQ(result, BigDecimal("0.0009765625"));

    a = BigDecimal("123");
    b = BigDecimal("8932483294839248234309483204802.38408499");
    Precision prec1(RoundType::RoundHalfEven, 50);
    result = a.Div(b, prec1);
    ASSERT_EQ(result, BigDecimal("1.376996697783508662080E-29"));

    a = BigDecimal("99999999999999999999999999999999999999999");
    b = BigDecimal("100000000");
    Precision prec2(RoundType::RoundHalfEven, 7);
    result = a.Div(b, prec2);
    ASSERT_EQ(result, BigDecimal("1e33"));

    a = BigDecimal("-5E+99999");
    b = BigDecimal("+5E-99999");
    result = a.Div(b);
    ASSERT_EQ(result, BigDecimal("-1e199998"));

    a = BigDecimal("0");
    b = BigDecimal("+9E+999999");
    result = a.Div(b);
    ASSERT_EQ(result, BigDecimal("0"));
}

TEST(DecimalTestSuite, TestException) {
    ASSERT_THROW(
        {
            BigDecimal a{"1.234567890123456789"};
            BigDecimal c = a.Div(BigDecimal("0"));
        },
        ssgx::exception_t::LocatedException);

    ASSERT_THROW(
        {
            BigDecimal a("4E+9999999999");
            BigDecimal b("100000000000000000");
            BigDecimal c = a.Multiply(b);
        },
        ssgx::exception_t::LocatedException);

    ASSERT_THROW({ BigDecimal a{"NaN"}; }, ssgx::exception_t::LocatedException);

    ASSERT_THROW({ BigDecimal a("+9E+99999999999"); }, ssgx::exception_t::LocatedException);

    ASSERT_THROW(
        {
            BigDecimal a("+9E+9999999999");
            BigDecimal b("+9E+9999999999");
            BigDecimal c = a.Multiply(b);
        },
        ssgx::exception_t::LocatedException);

    ASSERT_THROW(
        {
            BigDecimal a("+9E-9999999999");
            BigDecimal b("+9E+9999999999");
            BigDecimal c = a.Div(b);
        },
        ssgx::exception_t::LocatedException);

    ASSERT_THROW(
        {
            BigDecimal a("1");
            BigDecimal b("+9E+999999");
            BigDecimal c = a.Sub(b);
        },
        ssgx::exception_t::LocatedException);

    ASSERT_THROW(
        {
            BigDecimal a("1");
            BigDecimal b("3");
            BigDecimal c = a.Div(b);
        },
        ssgx::exception_t::LocatedException);

    ASSERT_NO_THROW({
        BigDecimal a("1");
        BigDecimal b("3");
        Precision prec(RoundType::RoundHalfUp, 3);
        BigDecimal result = a.Div(b, prec);
        ASSERT_EQ(result, BigDecimal("0.333"));
    });

    ASSERT_THROW({ Precision prec((RoundType)11, 3); }, ssgx::exception_t::LocatedException);

    ASSERT_THROW(
        {
            BigDecimal a("9E+999999");
            BigDecimal b("9E+999999");
            Precision prec2(RoundType::RoundHalfEven, -10);
            BigDecimal c = a.Multiply(b, prec2);
        },
        ssgx::exception_t::LocatedException);

    ASSERT_THROW(
        {
            Precision prec(RoundType::RoundHalfUp, 99);
            BigDecimal a("1");
            BigDecimal b("3");
            BigDecimal c = a.Div(b, prec);
        },
        ssgx::exception_t::LocatedException);

    ASSERT_THROW(
        {
            Precision prec(RoundType::RoundHalfUp, 98);
            BigDecimal a("100000000");
            BigDecimal b("3");
            BigDecimal c = a.Div(b, prec);
        },
        ssgx::exception_t::LocatedException);

    ASSERT_THROW(
        {
            BigDecimal a("+9E+100");
            BigDecimal b("0");
            BigDecimal c = a.Add(b);
        },
        ssgx::exception_t::LocatedException);

    ASSERT_THROW(
        {
            BigDecimal a("473958433492874928447939753.349287349287492844793492844793");
            BigDecimal b("3374927483274932843247424871473.479473824791794719473174913481");
            BigDecimal c = a.Multiply(b);
        },
        ssgx::exception_t::LocatedException);

    ASSERT_THROW(
        {
            Precision prec(RoundType::RoundHalfUp, 98);
            BigDecimal a("473958433492874928447939753.349287349287492844793492844793");
            BigDecimal b("3374927483274932843247424871473.479473824791794719473174913481");
            BigDecimal c = a.Multiply(b, prec);
        },
        ssgx::exception_t::LocatedException);

    ASSERT_THROW(
        {
            BigDecimal a("0");
            BigDecimal b("0");
            BigDecimal c = a.Div(b);
        },
        ssgx::exception_t::LocatedException);

    ASSERT_THROW({ Precision prec2(RoundType::RoundHalfEven, 99999999); }, ssgx::exception_t::LocatedException);

    ASSERT_THROW({ BigDecimal a(""); }, ssgx::exception_t::LocatedException);
}

TEST(DecimalTestSuite, TestRoundType) {
    BigDecimal a("1234504434.7777777781115");
    BigDecimal result;
    result = a.SetScale(10, RoundType::RoundUp);
    ASSERT_EQ(result, BigDecimal("1234504434.7777777782"));
    result = a.SetScale(10, RoundType::RoundDown);
    ASSERT_EQ(result, BigDecimal("1234504434.7777777781"));
    result = a.SetScale(10, RoundType::RoundCeiling);
    ASSERT_EQ(result, BigDecimal("1234504434.7777777782"));
    result = a.SetScale(10, RoundType::RoundFloor);
    ASSERT_EQ(result, BigDecimal("1234504434.7777777781"));
    result = a.SetScale(12, RoundType::RoundHalfUp);
    ASSERT_EQ(result, BigDecimal("1234504434.777777778112"));
    result = a.SetScale(12, RoundType::RoundHalfDown);
    ASSERT_EQ(result, BigDecimal("1234504434.777777778111"));
    result = a.SetScale(12, RoundType::RoundHalfEven);
    ASSERT_EQ(result, BigDecimal("1234504434.777777778112"));
    result = a.SetScale(-5, RoundType::Round05Up);
    ASSERT_EQ(result, BigDecimal("1234600000"));
    result = a.SetScale(8, RoundType::RoundTrunc);
    ASSERT_EQ(result, BigDecimal("1234504434.77777777"));

    BigDecimal b = a.Neg();
    result = b.SetScale(10, RoundType::RoundUp);
    ASSERT_EQ(result, BigDecimal("-1234504434.7777777782"));
    result = b.SetScale(10, RoundType::RoundDown);
    ASSERT_EQ(result, BigDecimal("-1234504434.7777777781"));
    result = b.SetScale(10, RoundType::RoundCeiling);
    ASSERT_EQ(result, BigDecimal("-1234504434.7777777781"));
    result = b.SetScale(10, RoundType::RoundFloor);
    ASSERT_EQ(result, BigDecimal("-1234504434.7777777782"));
    result = b.SetScale(12, RoundType::RoundHalfUp);
    ASSERT_EQ(result, BigDecimal("-1234504434.777777778112"));
    result = b.SetScale(12, RoundType::RoundHalfDown);
    ASSERT_EQ(result, BigDecimal("-1234504434.777777778111"));
    result = b.SetScale(12, RoundType::RoundHalfEven);
    ASSERT_EQ(result, BigDecimal("-1234504434.777777778112"));
    result = b.SetScale(-5, RoundType::Round05Up);
    ASSERT_EQ(result, BigDecimal("-1234600000"));
    result = b.SetScale(8, RoundType::RoundTrunc);
    ASSERT_EQ(result, BigDecimal("-1234504434.77777777"));

    ASSERT_EQ(BigDecimal("12.34").SetScale(1, RoundType::RoundUp), BigDecimal("12.4"));
    ASSERT_EQ(BigDecimal("-12.34").SetScale(1, RoundType::RoundUp), BigDecimal("-12.4"));

    ASSERT_EQ(BigDecimal("12.36").SetScale(1, RoundType::RoundDown), BigDecimal("12.3"));
    ASSERT_EQ(BigDecimal("-12.36").SetScale(1, RoundType::RoundDown), BigDecimal("-12.3"));

    ASSERT_EQ(BigDecimal("12.34").SetScale(1, RoundType::RoundCeiling), BigDecimal("12.4"));
    ASSERT_EQ(BigDecimal("-12.34").SetScale(1, RoundType::RoundCeiling), BigDecimal("-12.3"));

    ASSERT_EQ(BigDecimal("12.34").SetScale(1, RoundType::RoundFloor), BigDecimal("12.3"));
    ASSERT_EQ(BigDecimal("-12.34").SetScale(1, RoundType::RoundFloor), BigDecimal("-12.4"));

    ASSERT_EQ(BigDecimal("12.34").SetScale(1, RoundType::RoundHalfUp), BigDecimal("12.3"));
    ASSERT_EQ(BigDecimal("-12.34").SetScale(1, RoundType::RoundHalfUp), BigDecimal("-12.3"));
    ASSERT_EQ(BigDecimal("12.35").SetScale(1, RoundType::RoundHalfUp), BigDecimal("12.4"));
    ASSERT_EQ(BigDecimal("-12.35").SetScale(1, RoundType::RoundHalfUp), BigDecimal("-12.4"));
    ASSERT_EQ(BigDecimal("12.36").SetScale(1, RoundType::RoundHalfUp), BigDecimal("12.4"));
    ASSERT_EQ(BigDecimal("-12.36").SetScale(1, RoundType::RoundHalfUp), BigDecimal("-12.4"));

    ASSERT_EQ(BigDecimal("12.34").SetScale(1, RoundType::RoundHalfDown), BigDecimal("12.3"));
    ASSERT_EQ(BigDecimal("-12.34").SetScale(1, RoundType::RoundHalfDown), BigDecimal("-12.3"));
    ASSERT_EQ(BigDecimal("12.35").SetScale(1, RoundType::RoundHalfDown), BigDecimal("12.3"));
    ASSERT_EQ(BigDecimal("-12.35").SetScale(1, RoundType::RoundHalfDown), BigDecimal("-12.3"));
    ASSERT_EQ(BigDecimal("12.351").SetScale(1, RoundType::RoundHalfDown), BigDecimal("12.4"));
    ASSERT_EQ(BigDecimal("-12.351").SetScale(1, RoundType::RoundHalfDown), BigDecimal("-12.4"));

    ASSERT_EQ(BigDecimal("12.34").SetScale(1, RoundType::RoundHalfEven), BigDecimal("12.3"));
    ASSERT_EQ(BigDecimal("-12.34").SetScale(1, RoundType::RoundHalfEven), BigDecimal("-12.3"));
    ASSERT_EQ(BigDecimal("12.35").SetScale(1, RoundType::RoundHalfEven), BigDecimal("12.4"));
    ASSERT_EQ(BigDecimal("-12.35").SetScale(1, RoundType::RoundHalfEven), BigDecimal("-12.4"));
    ASSERT_EQ(BigDecimal("12.25").SetScale(1, RoundType::RoundHalfEven), BigDecimal("12.2"));
    ASSERT_EQ(BigDecimal("-12.25").SetScale(1, RoundType::RoundHalfEven), BigDecimal("-12.2"));
    ASSERT_EQ(BigDecimal("12.251").SetScale(1, RoundType::RoundHalfEven), BigDecimal("12.3"));
    ASSERT_EQ(BigDecimal("-12.251").SetScale(1, RoundType::RoundHalfEven), BigDecimal("-12.3"));

    ASSERT_EQ(BigDecimal("12.53").SetScale(1, RoundType::Round05Up), BigDecimal("12.6"));
    ASSERT_EQ(BigDecimal("-12.53").SetScale(1, RoundType::Round05Up), BigDecimal("-12.6"));
    ASSERT_EQ(BigDecimal("12.66").SetScale(1, RoundType::Round05Up), BigDecimal("12.6"));
    ASSERT_EQ(BigDecimal("-12.66").SetScale(1, RoundType::Round05Up), BigDecimal("-12.6"));

    ASSERT_EQ(BigDecimal("12.36").SetScale(1, RoundType::Round05Up), BigDecimal("12.3"));
    ASSERT_EQ(BigDecimal("-12.36").SetScale(1, RoundType::Round05Up), BigDecimal("-12.3"));
}

TEST(DecimalTestSuite, TestOther) {
    BigDecimal a;
    BigDecimal b;
    BigDecimal result;

    a = BigDecimal("+9E+999999");
    b = a.Neg();
    ASSERT_EQ(a.Add(b), BigDecimal("0"));

    a = BigDecimal("-88888888");
    b = a.Neg();
    ASSERT_EQ(a.Add(b), BigDecimal("0"));

    a = BigDecimal("+0");
    b = a.Neg();
    ASSERT_EQ(a.Add(b), BigDecimal("0"));

    a = BigDecimal("7480.758102003");
    ASSERT_EQ(a.ToStr(), std::string("7480.758102003"));
    ASSERT_EQ(a.ToStr(6, RoundType::RoundHalfEven), std::string("7480.758102"));
    ASSERT_EQ(BigDecimal(a.ToStr(-2, RoundType::RoundHalfEven).c_str()), BigDecimal("7500"));

    a = BigDecimal("1");
    b = BigDecimal("3");
    Precision prec1(RoundType::RoundHalfUp, 33);
    result = a.Div(b, prec1);
    ASSERT_EQ(result, BigDecimal("0.333333333333333333333333333333333"));

    a = BigDecimal("100000000");
    b = BigDecimal("3");
    Precision prec2(RoundType::RoundHalfUp, 19);
    result = a.Div(b, prec2);
    ASSERT_EQ(result.ToStr(), std::string("33333333.3333333333333333333"));
}
