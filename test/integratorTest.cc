#include "../src/integrator.hh"

#include <boost/test/unit_test.hpp>
#include <iostream>

namespace S2O {
namespace TEST {

const float ZERO = 0.0f;

struct IntegratorValidValuesFixture {
  IntegratorValidValuesFixture() {}

  ~IntegratorValidValuesFixture() { mIntegrator.Reset(); }

  Integrator mIntegrator;
  float mPositiveNormalValue = 123.45f;
  float mNegativeNormalValue = -123.45f;
  float mZeroValue = ZERO;
};

BOOST_FIXTURE_TEST_SUITE(Integrator_Valid_Input_Values,
                         IntegratorValidValuesFixture)

BOOST_AUTO_TEST_CASE(Positive_Values_Test) {
  std::cout << "starting integrator-test" << '\n';
  mIntegrator.Integrate(mPositiveNormalValue);
  mIntegrator.Integrate(mPositiveNormalValue);

  float integrated_value = mIntegrator.getIntegratedValue();

  std::string integrated_value_str = mIntegrator.getIntegratedValueAsStr(3);

  BOOST_REQUIRE(integrated_value > ZERO);
}

BOOST_AUTO_TEST_CASE(Negative_Values_Test) {
  mIntegrator.Integrate(mNegativeNormalValue);
  mIntegrator.Integrate(mNegativeNormalValue);

  float integrated_value = mIntegrator.getIntegratedValue();

  std::string integrated_value_str = mIntegrator.getIntegratedValueAsStr(3);

  BOOST_REQUIRE(integrated_value < ZERO);
}

BOOST_AUTO_TEST_CASE(Zero_Values_Test) {
  mIntegrator.Integrate(mZeroValue);
  mIntegrator.Integrate(mZeroValue);

  float integrated_value = mIntegrator.getIntegratedValue();

  std::string integrated_value_str = mIntegrator.getIntegratedValueAsStr(3);

  BOOST_REQUIRE(integrated_value == ZERO);
  BOOST_REQUIRE(integrated_value_str == "0.000");
}

BOOST_AUTO_TEST_SUITE_END()
} // namespace TEST
} // namespace S2O