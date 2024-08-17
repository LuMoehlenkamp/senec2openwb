#include "configManager.hh"

using namespace S2O;

const std::string ConfigManager::CONFIG_PATH = "./dat/params.json";

ConfigManager *ConfigManager::mpConfigManager = nullptr;
// clang-format off
ConfigManager::ConfigManager(const std::string &arFilePathAndName)
    : mConfigLoaded(false)
    , mFilePathAndName(arFilePathAndName)
    , mTestMode(DEFAULT_TESTMODE)
    , mSenecUpdateTime(DEFAULT_SENEC_UPDATE_TIME)
    , mOpenWbIp(DEFAULT_MQTT_BROKER_IP)
    , mOpenWbPort(DEFAULT_MQTT_BROKER_PORT)
    , mSenecTimeoutTime(DEFAULT_SENEC_TIMEOUT_MS) {
}
// clang-format on

ConfigManager *ConfigManager::GetInstance(const std::string arFilePathAndName) {
  if (mpConfigManager == nullptr)
    mpConfigManager = new ConfigManager(arFilePathAndName);
  return mpConfigManager;
}

boost::optional<bool> ConfigManager::GetTestMode() {
  EnsureConfigLoaded();
  if (mConfigLoaded)
    return mTestMode;
  return boost::none;
}

boost::optional<unsigned> ConfigManager::GetSenecUpdateTime() {
  EnsureConfigLoaded();
  if (mConfigLoaded)
    return mSenecUpdateTime;
  return boost::none;
}

boost::optional<std::string> ConfigManager::GetOpenWbIp() {
  EnsureConfigLoaded();
  if (mConfigLoaded)
    return mOpenWbIp;
  return boost::none;
}

boost::optional<unsigned> ConfigManager::GetOpenWbPort() {
  EnsureConfigLoaded();
  if (mConfigLoaded)
    return mOpenWbPort;
  return boost::none;
}

boost::optional<long> ConfigManager::GetSenecTimeoutTime() {
  EnsureConfigLoaded();
  if (mConfigLoaded)
    return mSenecTimeoutTime;
  return boost::none;
}

bool ConfigManager::LoadConfig() {
  try {
    boost::property_tree::ptree tree;
    boost::property_tree::read_json(mFilePathAndName, tree);
    mTestMode = tree.get(TEST_MODE_NAME, DEFAULT_TESTMODE);
    mSenecUpdateTime =
        tree.get(SENEC_UPDATE_TIME_NAME, DEFAULT_SENEC_UPDATE_TIME);
    mOpenWbIp = tree.get(MQTT_BROKER_IP_NAME, DEFAULT_MQTT_BROKER_IP);
    mOpenWbPort = tree.get(MQTT_BROKER_PORT_NAME, DEFAULT_MQTT_BROKER_PORT);
    mSenecTimeoutTime = tree.get(SENEC_TIMEOUT_NAME, DEFAULT_SENEC_TIMEOUT_MS);
  } catch (const boost::property_tree::json_parser_error &pt_e) {
    std::cerr << pt_e.what() << '\n';
    return false;
  } catch (const boost::property_tree::ptree_bad_path &pt_bp_e) {
    std::cerr << pt_bp_e.what() << '\n';
    return false;
  } catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
    return false;
  } catch (...) {
    return false;
  }
  return true;
}

void ConfigManager::ResetToDefaults() {
  mConfigLoaded = false;
  mFilePathAndName = "";
  mTestMode = DEFAULT_TESTMODE;
  mSenecUpdateTime = DEFAULT_SENEC_UPDATE_TIME;
  mOpenWbIp = DEFAULT_MQTT_BROKER_IP;
  mOpenWbPort = DEFAULT_MQTT_BROKER_PORT;
  mSenecTimeoutTime = DEFAULT_SENEC_TIMEOUT_MS;
  mpConfigManager = nullptr;
}

void ConfigManager::EnsureConfigLoaded() {
  if (mConfigLoaded)
    return;
  mConfigLoaded = LoadConfig();
}