#pragma once

#include <cstdlib>
#include <iostream>
#include <istream>

#include <boost/asio.hpp>
#include <boost/optional.hpp>
#include <boost/property_tree/ptree.hpp>

#include "integrator.hh"
#include "mqttPublisher.hh"

namespace S2O {

class SenecDataAcquisitionCurl {
public:
  SenecDataAcquisitionCurl(boost::asio::io_context &ioContext,
                           unsigned TimerDuration);
  SenecDataAcquisitionCurl(SenecDataAcquisitionCurl &&) = delete;
  SenecDataAcquisitionCurl(const SenecDataAcquisitionCurl &) = delete;
  void Acquire();

private:
  boost::asio::io_context &mrIoContext;
  unsigned mTimerDuration;
  boost::asio::steady_timer mTimer;
  boost::property_tree::basic_ptree<std::string, std::string> mTree;
  mqttPublisher mPublisher;
  Integrator mInverterExportedEnergy;
  Integrator mGridImportedEnergy;
  Integrator mGridExportedEnergy;
  Integrator mBatteryImportedEnergy;
  Integrator mBatteryExportedEnergy;
};

} // namespace S2O