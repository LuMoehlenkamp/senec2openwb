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

BOOST_AUTO_TEST_SUITE_END()
} // namespace TEST
} // namespace S2O