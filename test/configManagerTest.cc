#include "../src/configManager.hh"
#include "testDefines.hh"

#include <boost/property_tree/exceptions.hpp>
#include <boost/test/unit_test.hpp>
#include <iostream>

namespace S2O {
namespace TEST {

struct ConfigManagerTestValidFixture {
  ConfigManagerTestValidFixture() {
    mpConfigManager = S2O::ConfigManager::GetInstance(VALID_CONFIG_PATH);
  }

  ~ConfigManagerTestValidFixture() { mpConfigManager->ResetToDefaults(); }

  S2O::ConfigManager *mpConfigManager;
};

struct ConfigManagerTestInvalidFixture {
  ConfigManagerTestInvalidFixture() {
    mpConfigManager = S2O::ConfigManager::GetInstance(INVALID_CONFIG_PATH);
  }

  ~ConfigManagerTestInvalidFixture() { mpConfigManager->ResetToDefaults(); }

  S2O::ConfigManager *mpConfigManager;
};

BOOST_FIXTURE_TEST_SUITE(ConfigManagerTest_Valid, ConfigManagerTestValidFixture)

BOOST_AUTO_TEST_CASE(config_start) {
  std::cout << "starting config-manager-test" << '\n';
}

BOOST_AUTO_TEST_CASE(
    GetInstance_ValidConfigPath_ReturnsValidPointerAndExpectedParameters) {
  bool expected_testmode(true);
  unsigned expected_senec_update_time = 3000U;
  std::string expected_open_wb_ip = "192.168.178.20";
  unsigned expected_open_wb_port = 1883;

  boost::optional<bool> testmode = mpConfigManager->GetTestMode();
  auto senec_update_time = mpConfigManager->GetSenecUpdateTime();
  auto open_wb_ip = mpConfigManager->GetOpenWbIp();
  auto open_wb_port = mpConfigManager->GetOpenWbPort();

  BOOST_REQUIRE(mpConfigManager != nullptr);
  BOOST_REQUIRE(testmode.is_initialized());
  BOOST_CHECK_EQUAL(expected_testmode, testmode.get());
  BOOST_CHECK_EQUAL(expected_senec_update_time, senec_update_time.get());
  BOOST_CHECK_EQUAL(expected_open_wb_ip, open_wb_ip.get());
  BOOST_CHECK_EQUAL(expected_open_wb_port, open_wb_port.get());
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_FIXTURE_TEST_SUITE(ConfigManagerTest_Invalid,
                         ConfigManagerTestInvalidFixture)

BOOST_AUTO_TEST_CASE(GetInstance_InvalidPath_DoesNotThrow) {
  BOOST_REQUIRE(mpConfigManager != nullptr);

  boost::optional<bool> testmode;
  BOOST_REQUIRE_NO_THROW(testmode = mpConfigManager->GetTestMode());
  BOOST_CHECK(!testmode.is_initialized());

  boost::optional<unsigned> senec_update_time;
  BOOST_REQUIRE_NO_THROW(senec_update_time =
                             mpConfigManager->GetSenecUpdateTime());
  BOOST_CHECK(!senec_update_time.is_initialized());
}

BOOST_AUTO_TEST_CASE(config_end) {
  std::cout << "finished config-manager-test" << '\n';
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace TEST
} // namespace S2O