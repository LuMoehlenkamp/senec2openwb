#pragma once

#include "conversion.hh"
#include "dataAcquisitionDefines.hh"
#include "integrator.hh"
#include "mqttPublisher.hh"

#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <curl/curl.h>
#include <string>

namespace S2O {
class SenecDataAcquisitionLibCurl {
private:
  CURL *mCurl;
  boost::asio::io_context &mrIoContext;
  unsigned mTimerDuration;
  boost::asio::steady_timer mTimer;
  ptree mTree;
  std::string mUrl{"https://192.168.178.40/lala.cgi"};
  std::string mPostData{"{\" ENERGY \":{},\" PM1OBJ1 "
                        "\":{},\" PM1OBJ2 \":{},\" RTC "
                        "\":{}}"};
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

public:
  SenecDataAcquisitionLibCurl(boost::asio::io_context &ioContext,
                              unsigned TimerDuration);
  ~SenecDataAcquisitionLibCurl();
  SenecDataAcquisitionLibCurl(SenecDataAcquisitionLibCurl &&) = delete;
  SenecDataAcquisitionLibCurl(const SenecDataAcquisitionLibCurl &) = delete;
};

// ToDo: move implementation to cc file

} // namespace S2O