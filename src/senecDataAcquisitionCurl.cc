#include "senecDataAcquisitionCurl.hh"
#include "conversion.hh"

#include <chrono>

#include <boost/bind/bind.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <cmath>
#include <string>
#include <vector>

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
    // std::cout << "issuing curl command." << '\n';
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
      std::time_t time_s_epoch = static_cast<std::time_t>(timestamp);
      std::stringstream ss;
      ss << std::put_time(std::localtime(&time_s_epoch), "%F %T.\n");
      mPublisher.publishTime(ss.str()); // todo: rework this method
    }

    // openWB/set/pv/1/W PV-Erzeugungsleistung in Watt, int, positiv
    // openWB/set/pv/1/WhCounter Erzeugte Energie in Wh, float, nur positiv // ToDo: add integrator
    const std::string topic_inv_power_str("openWB/set/pv/1/W");
    const std::string senec_inv_power_str("ENERGY.GUI_INVERTER_POWER");
    std::string inv_power_raw_str = mTree.get<std::string>(senec_inv_power_str); // todo: invert this number // fl
    std::string inv_power_pub_str;
    Conversion::ConvertToString(inv_power_raw_str, inv_power_pub_str, false, false);
    mPublisher.publishStr(topic_inv_power_str, inv_power_pub_str);

    // openWB/set/evu/WhImported Bezogene Energie in Wh, float, Punkt als Trenner, nur positiv
    // openWB/set/evu/WhExported Eingespeiste Energie in Wh, float, Punkt als Trenner, nur positiv


    // openWB/set/evu/PfPhase1 Powerfaktor für Phase 1, float, Punkt als Trenner, positiv Bezug, negativ Einspeisung
    // openWB/set/evu/PfPhase2 Powerfaktor für Phase 2, float, Punkt als Trenner, positiv Bezug, negativ Einspeisung
    // openWB/set/evu/PfPhase3 Powerfaktor für Phase 3, float, Punkt als Trenner, positiv Bezug, negativ Einspeisung

    // openWB/set/evu/W Bezugsleistung in Watt, int, positiv Bezug, negativ Einspeisung                                   -> done, float->int
    const std::string topic_grid_power_str("openWB/set/evu/W");
    const std::string senec_grid_power_str("ENERGY.GUI_GRID_POW");
    std::string grid_power_raw_str = mTree.get<std::string>(senec_grid_power_str);
    std::string grid_power_pub_str;
    Conversion::ConvertToString(grid_power_raw_str, grid_power_pub_str);
    mPublisher.publishStr(topic_grid_power_str, grid_power_pub_str);

    // openWB/set/evu/HzFrequenz oder openWB/set/evu/Hz Netzfrequenz in Hz, float, Punkt als Trenner                      -> done, float->float
    const std::string topic_freq_str("openWB/set/evu/HzFrequenz");
    const std::string senec_freq_str("PM1OBJ1.FREQ");
    std::string frequency_raw_str = mTree.get<std::string>(senec_freq_str);
    std::string frequency_pub_str;
    Conversion::ConvertToString(frequency_raw_str, frequency_pub_str, false, true);
    mPublisher.publishStr(topic_freq_str, frequency_pub_str);

    // openWB/set/evu/WPhase1 Leistung in Watt für Phase 1, float, Punkt als Trenner, positiv Bezug, negativ Einspeisung
    // openWB/set/evu/WPhase2 Leistung in Watt für Phase 2, float, Punkt als Trenner, positiv Bezug, negativ Einspeisung
    // openWB/set/evu/WPhase3 Leistung in Watt für Phase 3, float, Punkt als Trenner, positiv Bezug, negativ Einspeisung

    std::vector<std::string> topic_grid_powers_str_vec{"openWB/set/evu/WPhase1", "openWB/set/evu/WPhase2", "openWB/set/evu/WPhase3"};
    const std::string senec_powers_str("PM1OBJ1.P_AC");
    std::vector<std::string> powers_raw_str_vec;
    std::vector<std::string> powers_pub_str_vec{"","",""};
    for (boost::property_tree::ptree::value_type &power : mTree.get_child(senec_powers_str)) // fl
    {
      powers_raw_str_vec.push_back(power.second.data());
    }
    for (auto topics_it = topic_grid_powers_str_vec.begin(), raw_it = powers_raw_str_vec.begin(), pub_it = powers_pub_str_vec.begin();
         raw_it != powers_raw_str_vec.end();
         ++topics_it, ++raw_it, ++pub_it)
    {
      Conversion::ConvertToString(*raw_it, *pub_it, false, true);
      mPublisher.publishStr(*topics_it, *pub_it);
    }

    // openWB/set/evu/VPhase1 Spannung in Volt für Phase 1, float, Punkt als Trenner
    // openWB/set/evu/VPhase2 Spannung in Volt für Phase 2, float, Punkt als Trenner
    // openWB/set/evu/VPhase3 Spannung in Volt für Phase 3, float, Punkt als Trenner
    std::vector<std::string> topic_grid_volts_str_vec{"openWB/set/evu/VPhase1", "openWB/set/evu/VPhase2", "openWB/set/evu/VPhase3"};
    const std::string senec_volts_str("PM1OBJ1.U_AC");
    std::vector<std::string> volts_raw_str_vec;
    std::vector<std::string> volts_pub_str_vec{"","",""};
    for (boost::property_tree::ptree::value_type &voltage : mTree.get_child(senec_volts_str)) // fl
    {
      volts_raw_str_vec.push_back(voltage.second.data());
    }
    for (auto topics_it = topic_grid_volts_str_vec.begin(), raw_it = volts_raw_str_vec.begin(), pub_it = volts_pub_str_vec.begin();
         raw_it != volts_raw_str_vec.end();
         ++topics_it, ++raw_it, ++pub_it)
    {
      Conversion::ConvertToString(*raw_it, *pub_it, false, true);
      mPublisher.publishStr(*topics_it, *pub_it);
    }

    // openWB/set/evu/APhase1 Strom in Ampere für Phase 1, float, Punkt als Trenner, positiv Bezug, negativ Einspeisung
    // openWB/set/evu/APhase2 Strom in Ampere für Phase 2, float, Punkt als Trenner, positiv Bezug, negativ Einspeisung
    // openWB/set/evu/APhase3 Strom in Ampere für Phase 3, float, Punkt als Trenner, positiv Bezug, negativ Einspeisung
    std::vector<std::string> topic_grid_amps_str_vec{"openWB/set/evu/APhase1", "openWB/set/evu/APhase2", "openWB/set/evu/APhase3"};
    const std::string senec_amps_str("PM1OBJ1.I_AC");
    std::vector<std::string> amps_raw_str_vec;
    std::vector<std::string> amps_pub_str_vec{"","",""};
    for (boost::property_tree::ptree::value_type &current : mTree.get_child(senec_amps_str)) // fl
    {
      amps_raw_str_vec.push_back(current.second.data());
    }
    for (auto topics_it = topic_grid_amps_str_vec.begin(), raw_it = amps_raw_str_vec.begin(), pub_it = amps_pub_str_vec.begin(), powers_it = powers_pub_str_vec.begin();
         raw_it != amps_raw_str_vec.end();
         ++topics_it, ++raw_it, ++pub_it, ++powers_it)
    {
      Conversion::ConvertToString(*raw_it, *pub_it, std::signbit(std::stof(*powers_it)), true);
      mPublisher.publishStr(*topics_it, *pub_it);
    }

    // std::string house_power = mTree.get<std::string>("ENERGY.GUI_HOUSE_POW"); // fl

    // openWB/set/houseBattery/WhImported Geladene Energie in Wh, float, nur positiv
    // openWB/set/houseBattery/WhExported Entladene Energie in Wh, float, nur positiv

    // openWB/set/houseBattery/W Speicherleistung in Watt, int, positiv Ladung, negativ Entladung   -> done
    const std::string topic_bat_power_str("openWB/set/houseBattery/W");
    const std::string senec_bat_power_str("ENERGY.GUI_BAT_DATA_POWER");
    std::string bat_power_raw_str = mTree.get<std::string>(senec_bat_power_str);
    std::string bat_power_pub_str;
    Conversion::ConvertToString(bat_power_raw_str, bat_power_pub_str);
    mPublisher.publishStr(topic_bat_power_str, bat_power_pub_str);

    // openWB/set/houseBattery/%Soc Ladestand des Speichers, int, 0-100
    const std::string topic_bat_soc_str("openWB/set/houseBattery/%Soc");
    const std::string senec_bat_soc_str("ENERGY.GUI_BAT_DATA_FUEL_CHARGE");
    std::string bat_soc_raw_str = mTree.get<std::string>(senec_bat_soc_str);
    std::string bat_soc_pub_str;
    Conversion::ConvertToString(bat_soc_raw_str, bat_soc_pub_str);
    mPublisher.publishStr(topic_bat_soc_str, bat_soc_pub_str);

  } catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
  }
}

// clang-format on