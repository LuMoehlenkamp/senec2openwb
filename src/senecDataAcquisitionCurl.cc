#include "senecDataAcquisitionCurl.hh"
#include "conversion.hh"

#include <chrono>

#include <boost/bind/bind.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <string>

using namespace boost;
using namespace boost::asio;
using namespace boost::placeholders;
using namespace S2O;

// clang-format off
SenecDataAcquisitionCurl::SenecDataAcquisitionCurl(io_context &ioContext,
                                                   unsigned int TimerDuration)
  : mrIoContext(ioContext)
  , mTimerDuration(TimerDuration)
  , mTimer(ioContext, std::chrono::seconds(INITIAL_TIMER_DURATION))
{
  mTimer.async_wait(bind(&SenecDataAcquisitionCurl::Acquire, this));
}

void SenecDataAcquisitionCurl::Acquire()
{
  try
  {
    std::system("./dat/curl_command");
    // std::cout << std::ifstream("curl_test.txt").rdbuf();

    mTimer.expires_after(std::chrono::seconds(mTimerDuration));
    mTimer.async_wait(boost::bind(&SenecDataAcquisitionCurl::Acquire, this));
  }
  catch(const std::exception& e)
  {
    std::cerr << e.what() << '\n';
  }
  boost::property_tree::basic_ptree<std::string, std::string> tree;

  try {
    boost::property_tree::read_json("./dat/curl_cmd_response", tree);
    std::string time = tree.get<std::string>("RTC.WEB_TIME");
    ConversionResultOpt time_cr = Conversion::Convert(time);
    if (time_cr.is_initialized())
    {
      unsigned x = reinterpret_cast<unsigned &>(time_cr.get());
      std::cout << "time: " << time_cr.get() << '\n';
      std::chrono::seconds time_sec(x);
      // std::chrono::_V2::system_clock::time_point time_tp(time_sec);
      // std::cout << "time (chrono): " << time_tp.time_since_epoch(). << '\n';
    }
  } catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
  }

}

// clang-format on