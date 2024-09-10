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
  const std::string mTopicInvPower{"openWB/set/pv/1/W"};
  const std::string mTopicInvEnergy{"openWB/set/pv/1/WhCounter"};

  const std::string mTreeElemGridPower{"ENERGY.GUI_GRID_POW"};
  const std::string mTopicGridPower{"openWB/set/evu/W"};
  const std::string mTopicGridImportedEnergy{"openWB/set/evu/WhImported"};
  const std::string mTopicGridExportedEnergy{"openWB/set/evu/WhExported"};
  const std::string mTreeElemFreq{"PM1OBJ1.FREQ"};
  const std::string mTopicFrequency{"openWB/set/evu/HzFrequenz"};
  const std::string mTreeElemGridPowers{"PM1OBJ1.P_AC"};
  std::vector<std::string> mTopicGridPowersVec{"openWB/set/evu/WPhase1",
                                               "openWB/set/evu/WPhase2",
                                               "openWB/set/evu/WPhase3"};
  std::array<float, 3> mPowerValues{0.0f, 0.0f, 0.0f};
  const std::string mTreeElemVoltages{"PM1OBJ1.U_AC"};
  std::vector<std::string> mTopicGridVoltagesVec{"openWB/set/evu/VPhase1",
                                                 "openWB/set/evu/VPhase2",
                                                 "openWB/set/evu/VPhase3"};
  std::array<float, 3> mVoltageValues{0.0f, 0.0f, 0.0f};
  const std::string mTreeElemCurrents{"PM1OBJ1.I_AC"};
  std::vector<std::string> mTopicGridCurrentsVec{"openWB/set/evu/APhase1",
                                                 "openWB/set/evu/APhase2",
                                                 "openWB/set/evu/APhase3"};
  std::array<float, 3> mCurrentValues{0.0f, 0.0f, 0.0f};
  std::vector<std::string> mTopicGridPowerFactorVec{"openWB/set/evu/PfPhase1",
                                                    "openWB/set/evu/PfPhase2",
                                                    "openWB/set/evu/PfPhase3"};
  const std::string mPowerFactorValue{"1.0"};

  const std::string mTreeElemBatteryPower{"ENERGY.GUI_BAT_DATA_POWER"};
  const std::string mTopicBatteryPower{"openWB/set/houseBattery/W"};
  const std::string mTopicBatteryImportedEnergy{
      "openWB/set/houseBattery/WhImported"};
  const std::string mTopicBatteryExportedEnergy{
      "openWB/set/houseBattery/WhExported"};
  const std::string mTopicBatterySoc{"openWB/set/houseBattery/%Soc"};
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