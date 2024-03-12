#pragma once

// #include "dataAcquisitionDefines.hh"
// #include "global.hh"
// #include "senecResultSubject.hh"

#include <cstdlib>
#include <iostream>
#include <istream>

#include <boost/asio.hpp>
#include <boost/optional.hpp>

namespace S2O {

// clang-format off
const std::string CURL_REQUEST =
    "curl -sk https://192.168.178.40/lala.cgi -H"
    " 'Content-Type: application/json' -d '{"
      " \"ENERGY\":{\"GUI_HOUSE_POW\":"
                  ",\"GUI_GRID_POW\":"
                  ",\"GUI_INVERTER_POWER\":"
                  ",\"GUI_BAT_DATA_POWER\":"
                  ",\"GUI_BAT_DATA_FUEL_CHARGE\":"
                  ",\"STAT_STATE\":"
                  " }, "
      " \"STATISTIC\":{\"LIVE_GRID_IMPORT\":"
                  "}, "
      " \"PM1OBJ1\":{\"P_AC\":},"
      " \"RTC\" : {}"
    "}' > curl_test.txt";
// clang-format off

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
};

} // namespace S2O