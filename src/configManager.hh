#pragma once

#include <exception>
#include <iostream>
#include <set>
#include <string>

#include <boost/foreach.hpp>
#include <boost/optional.hpp>
#include <boost/property_tree/exceptions.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

namespace S2O {

class ConfigManager {
private:
  const bool DEFAULT_TESTMODE = true;
  const unsigned DEFAULT_SENEC_UPDATE_TIME = 10U;
  const std::string DEFAULT_MQTT_BROKER_IP = "192.168.178.20";
  const unsigned DEFAULT_MQTT_BROKER_PORT = 1883U;

  const std::string TEST_MODE_NAME = "testMode";
  const std::string SENEC_UPDATE_TIME_NAME = "senecUpdateTime_sec";
  const std::string MQTT_BROKER_IP_NAME = "openWb_ip";
  const std::string MQTT_BROKER_PORT_NAME = "openWb_port";

protected:
  ConfigManager(const std::string &arFilePathAndName);

  static ConfigManager *mpConfigManager;

public:
  static const std::string CONFIG_PATH;
  ConfigManager(ConfigManager &arOther) = delete;
  void operator=(const ConfigManager &arOther) = delete;

  static ConfigManager *GetInstance(const std::string arFilePathAndName);

  boost::optional<bool> GetTestMode();
  boost::optional<unsigned> GetSenecUpdateTime();

  boost::optional<std::string> GetOpenWbIp();
  boost::optional<unsigned> GetOpenWbPort();

  bool LoadConfig();
  void ResetToDefaults();

private:
  bool mConfigLoaded;
  std::string mFilePathAndName;
  bool mTestMode;
  unsigned mSenecUpdateTime;
  std::string mOpenWbIp;
  unsigned mOpenWbPort;

  void EnsureConfigLoaded();
};

} // namespace S2O
