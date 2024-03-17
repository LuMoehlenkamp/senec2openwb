#pragma once

// #include "dataAcquisitionDefines.hh"
// #include "global.hh"
// #include "senecResultSubject.hh"

#include <cstdlib>
#include <iostream>
#include <istream>

#include <boost/asio.hpp>
#include <boost/optional.hpp>
#include <boost/property_tree/ptree.hpp>

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
};

} // namespace S2O