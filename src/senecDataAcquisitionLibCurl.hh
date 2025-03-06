#pragma once

#include "conversion.hh"
#include "dataAcquisitionDefines.hh"
#include "integrator.hh"
#include "mqttPublisher.hh"

#include <array>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <curl/curl.h>
#include <string>
#include <vector>

namespace S2O {

// clang-format off
class SenecDataAcquisitionLibCurl {
private:
  CURL *mCurl;
  boost::asio::io_context &mrIoContext;
  unsigned mTimerDuration_ms;
  long mTimeoutDuration_ms;
  long mConnectTimeoutDuration_ms;
  boost::asio::steady_timer mTimer;
  ptree mTree;

  const std::string mUrl{"https://192.168.178.40/lala.cgi"};
  const std::string mPostData{"{\" ENERGY \":{},\" PM1OBJ1 "
                              "\":{},\" PM1OBJ2 \":{},\" RTC "
                              "\":{}}"};

  const std::string mTreeElemRtcOffset{"RTC.UTC_OFFSET"};
  const std::string mTreeElemWebTime{"RTC.WEB_TIME"};

  const std::string mTreeElemInvPower{"ENERGY.GUI_INVERTER_POWER"};
  const std::string mTopicInvPower{"openWB/set/pv/2/get/power"};
  const std::string mTopicInvEnergy{"openWB/set/pv/2/get/exported "};

  const std::string mTreeElemGridPower{"ENERGY.GUI_GRID_POW"};
  const std::string mTopicGridPower{"openWB/set/counter/0/get/power"};
  const std::string mTopicGridImportedEnergy{"openWB/set/counter/0/get/imported"};
  const std::string mTopicGridExportedEnergy{"openWB/set/counter/0/get/exported"};
  const std::string mTreeElemFreq{"PM1OBJ1.FREQ"};
  const std::string mTopicFrequency{"openWB/set/counter/0/get/frequency"};
  const std::string mTreeElemGridPowers{"PM1OBJ1.P_AC"};
  std::string mTopicGridPowers{"openWB/set/counter/0/get/powers"};
  std::array<float, 3> mPowerValues{0.0f, 0.0f, 0.0f};
  const std::string mTreeElemVoltages{"PM1OBJ1.U_AC"};
  std::string mTopicGridVoltages{"openWB/set/counter/0/get/voltages"};
  std::array<float, 3> mVoltageValues{0.0f, 0.0f, 0.0f};
  const std::string mTreeElemCurrents{"PM1OBJ1.I_AC"};
  std::string mTopicGridCurrents{"openWB/set/counter/0/get/currents"};
  std::array<float, 3> mCurrentValues{0.0f, 0.0f, 0.0f};
  std::string mTopicGridPowerFactor{"openWB/set/counter/0/get/power_factors"};
  const std::string mPowerFactorValue{"1.0"};

  const std::string mTreeElemBatteryPower{"ENERGY.GUI_BAT_DATA_POWER"};
  const std::string mTopicBatteryPower{"openWB/set/bat/1/get/power"};
  const std::string mTopicBatteryImportedEnergy{"openWB/set/bat/1/get/imported"};
  const std::string mTopicBatteryExportedEnergy{"openWB/set/bat/1/get/exported"};
  const std::string mTopicBatterySoc{"openWB/set/bat/1/get/soc"};
  const std::string mTreeElemBatterySoc{"ENERGY.GUI_BAT_DATA_FUEL_CHARGE"};

  mqttPublisher mPublisher;
  Integrator mInverterExportedEnergy;
  Integrator mGridImportedEnergy;
  Integrator mGridExportedEnergy;
  Integrator mBatteryImportedEnergy;
  Integrator mBatteryExportedEnergy;

  void setTimerDuration();
  void Init();
  void Acquire();
  static size_t WriteCallback(void *contents, size_t size, size_t nmemb,
                              std::string *buffer);
  void ParseResponse(const std::string &response);
  void ProcessData();
  void ProcessTimeInformation();
  void ProcessInverterData();
  void ProcessGridData();
  void ProcessBatteryData();

public:
  SenecDataAcquisitionLibCurl(boost::asio::io_context &ioContext,
                              unsigned TimerDuration, long TimeoutDuration_ms,
                              long ConnectTimeoutDuration_ms);
  ~SenecDataAcquisitionLibCurl();
  SenecDataAcquisitionLibCurl(SenecDataAcquisitionLibCurl &&) = delete;
  SenecDataAcquisitionLibCurl(const SenecDataAcquisitionLibCurl &) = delete;
};

} // namespace S2O