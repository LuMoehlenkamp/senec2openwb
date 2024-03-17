#include "senecDataAcquisitionCurl.hh"
#include "conversion.hh"

#include <chrono>

#include <boost/bind/bind.hpp>
#include <boost/property_tree/json_parser.hpp>
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
  , mPublisher()
{
  mTimer.async_wait(bind(&SenecDataAcquisitionCurl::Acquire, this));
}

void SenecDataAcquisitionCurl::Acquire()
{
  mTree.clear();
  try
  {
    // ToDo: check path existence, non existend -> exit
    std::system("./dat/curl_command.sh");
    // std::cout << std::ifstream("curl_test.txt").rdbuf();

    mTimer.expires_after(std::chrono::seconds(mTimerDuration));
    mTimer.async_wait(boost::bind(&SenecDataAcquisitionCurl::Acquire, this));
  }
  catch(const std::exception& e)
  {
    std::cerr << e.what() << '\n';
  }

  try {
    boost::property_tree::read_json("./res/curl_cmd_response.json", mTree);

    std::string utc_offset = mTree.get<std::string>("RTC.UTC_OFFSET");
    ConversionResultOpt utc_offset_cr = Conversion::Convert(utc_offset);

    std::string time = mTree.get<std::string>("RTC.WEB_TIME");

    ConversionResultOpt time_cr = Conversion::Convert(time);
    if (time_cr.is_initialized() && utc_offset_cr.is_initialized())
    {
      auto utc_offset = boost::get<int>(utc_offset_cr.get());
      std::chrono::minutes offset_minutes(utc_offset);
      std::chrono::seconds offset_seconds(offset_minutes);

      auto timestamp = boost::get<uint>(time_cr.get());
      std::time_t time_s_epoch = static_cast<std::time_t>(timestamp - offset_seconds.count());
      std::stringstream ss;
      ss << std::put_time(std::localtime(&time_s_epoch), "%F %T.\n");

      // Print the formatted timestamp
      std::cout << "Timestamp: " << ss.str() << '\n';

      mPublisher.publishTime(ss.str());
    }

    std::string battery_soc = mTree.get<std::string>("ENERGY.GUI_BAT_DATA_FUEL_CHARGE");
    ConversionResultOpt battery_soc_cr = Conversion::Convert(battery_soc);
    if (battery_soc_cr.is_initialized())
    {
      auto bat_soc = boost::get<float>(battery_soc_cr.get());
      std::cout << "SOC: " << bat_soc << std::endl;
      std::cout << "SOC (str): " << std::to_string(bat_soc) << "\n";

      mPublisher.publishFloat("openWB/housebattery/%Soc", bat_soc);
    }
  } catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
  }
}

// clang-format on