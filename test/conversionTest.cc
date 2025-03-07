#include "../src/conversion.hh"

#include <boost/test/unit_test.hpp>
#include <iostream>

namespace S2O {
namespace TEST {

struct ConversionTestFixture {
  ConversionTestFixture() {}
  ~ConversionTestFixture() {}

  float mTolerance{0.0001f};
};

BOOST_FIXTURE_TEST_SUITE(Integrator_Valid_Input_Values, ConversionTestFixture)

BOOST_AUTO_TEST_CASE(
    ConvertStringToValue_StringIsConvertible_ConvertsAsExpected) {
  std::cout << "starting conversion-test" << '\n';
  std::string pi_str("3.1415");
  float expected{3.1415f};
  float value;

  Conversion::ConvertToFloatVal(pi_str, value);

  BOOST_CHECK_CLOSE(expected, value, mTolerance);
}

BOOST_AUTO_TEST_CASE(
    ConvertStringToValue_StringIsNotConvertible_ConvertsToZero) {
  std::string error_str("abc1");
  float expected{0.0f};
  float value;

  Conversion::ConvertToFloatVal(error_str, value);

  BOOST_CHECK_CLOSE(expected, value, mTolerance);
}

BOOST_AUTO_TEST_CASE(floatToString_precission_2_returns_as_expected) {
  float fl_value{3.1415f};
  std::string expected{"3.14"};
  std::string result;

  result = Conversion::floatToString(fl_value, 2);

  BOOST_CHECK_EQUAL(result, expected);
}

BOOST_AUTO_TEST_CASE(floatToString_precission_1_returns_as_expected) {
  float fl_value{3.1415f};
  std::string expected{"3.1"};
  std::string result;

  result = Conversion::floatToString(fl_value, 1);

  BOOST_CHECK_EQUAL(result, expected);
}

BOOST_AUTO_TEST_CASE(floatToString_precission_0_returns_as_expected) {
  float fl_value{3.1415f};
  std::string expected{"3"};
  std::string result;

  result = Conversion::floatToString(fl_value, 0);

  BOOST_CHECK_EQUAL(result, expected);
}

BOOST_AUTO_TEST_CASE(doubleToString_precission_2_returns_as_expected) {
  double fl_value{3.1415f};
  std::string expected{"3.14"};
  std::string result;

  result = Conversion::doubleToString(fl_value, 2);

  BOOST_CHECK_EQUAL(result, expected);
}

BOOST_AUTO_TEST_CASE(doubleToString_precission_1_returns_as_expected) {
  double fl_value{3.1415f};
  std::string expected{"3.1"};
  std::string result;

  result = Conversion::doubleToString(fl_value, 1);

  BOOST_CHECK_EQUAL(result, expected);
}

BOOST_AUTO_TEST_CASE(doubleToString_precission_0_returns_as_expected) {
  double fl_value{3.1415f};
  std::string expected{"3"};
  std::string result;

  result = Conversion::doubleToString(fl_value, 0);

  BOOST_CHECK_EQUAL(result, expected);
}

BOOST_AUTO_TEST_CASE(
    ConvertToString_ValidFloatInputNoInversionTwoDecimals_ReturnsAsExpected) {
  std::string InVal{"fl_42481EB8"};
  std::string result;
  std::string expected{"50.03"};

  Conversion::ConvertToString(InVal, result, false, 2);

  BOOST_CHECK_EQUAL(result, expected);
}

BOOST_AUTO_TEST_CASE(
    ConvertToString_ValidFloatInputWithInversionTwoDecimals_ReturnsAsExpected) {
  std::string InVal{"fl_42481EB8"};
  std::string result;
  std::string expected{"-50.03"};

  Conversion::ConvertToString(InVal, result, true, 2);

  BOOST_CHECK_EQUAL(result, expected);
}

BOOST_AUTO_TEST_CASE(
    ConvertToString_ValidFloatInputNoInversionOneDecimal_ReturnsAsExpected) {
  std::string InVal{"fl_42481EB8"};
  std::string result;
  std::string expected{"50.0"};

  Conversion::ConvertToString(InVal, result, false, 1);

  BOOST_CHECK_EQUAL(result, expected);
}

BOOST_AUTO_TEST_CASE(
    ConvertToString_ValidIntInputWithoutInversionZeroDecimals_ReturnsAsExpected) {
  std::string InVal{"i3_0000003C"};
  std::string result;
  std::string expected{"60"};

  Conversion::ConvertToString(InVal, result, false, 0);

  BOOST_CHECK_EQUAL(result, expected);
}

BOOST_AUTO_TEST_CASE(
    ConvertToString_ValidIntInputWithInversionZeroDecimals_ReturnsAsExpected) {
  std::string InVal{"i3_0000003C"};
  std::string result;
  std::string expected{"-60"};

  Conversion::ConvertToString(InVal, result, true, 0);

  BOOST_CHECK_EQUAL(result, expected);
}

BOOST_AUTO_TEST_CASE(conversion_end) {
  std::cout << "finished conversion-test" << '\n';
}

BOOST_AUTO_TEST_SUITE_END()
} // namespace TEST
} // namespace S2O