#include "senecDataAcquisitionCurl.hh"
#include "conversion.hh"

#include <chrono>

#include <boost/bind/bind.hpp>
#include <boost/property_tree/json_parser.hpp>
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
      // std::time_t time_s_epoch = static_cast<std::time_t>(timestamp - offset_seconds.count());
      std::time_t time_s_epoch = static_cast<std::time_t>(timestamp);
      std::stringstream ss;
      ss << std::put_time(std::localtime(&time_s_epoch), "%F %T.\n");
      // Print the formatted timestamp
      // std::cout << "Timestamp: " << ss.str() << '\n';
      mPublisher.publishTime(ss.str()); // todo: rework this method
    }

    // openWB/set/pv/1/W PV-Erzeugungsleistung in Watt, int, positiv
    // openWB/set/pv/1/WhCounter Erzeugte Energie in Wh, float, nur positiv // ToDo: add integrator
    const std::string mosq_inv_power_str("openWB/set/pv/1/W");
    std::string inverter_power = mTree.get<std::string>("ENERGY.GUI_INVERTER_POWER"); // todo: invert this number // fl
    ConversionResultOpt inverter_power_cr = Conversion::Convert(inverter_power);
    if (inverter_power_cr.is_initialized())
    {
      auto inv_pow = -1.0f * boost::get<float>(inverter_power_cr.get());
      // std::cout << "inverter power: " << inv_pow << '\n';
      mPublisher.publishInt(mosq_inv_power_str, static_cast<int>(inv_pow));
    }

    // openWB/set/evu/W Bezugsleistung in Watt, int, positiv Bezug, negativ Einspeisung                                   -> done
    // openWB/set/evu/APhase1 Strom in Ampere für Phase 1, float, Punkt als Trenner, positiv Bezug, negativ Einspeisung
    // openWB/set/evu/APhase2 Strom in Ampere für Phase 2, float, Punkt als Trenner, positiv Bezug, negativ Einspeisung
    // openWB/set/evu/APhase3 Strom in Ampere für Phase 3, float, Punkt als Trenner, positiv Bezug, negativ Einspeisung
    // openWB/set/evu/WhImported Bezogene Energie in Wh, float, Punkt als Trenner, nur positiv
    // openWB/set/evu/WhExported Eingespeiste Energie in Wh, float, Punkt als Trenner, nur positiv
    // openWB/set/evu/VPhase1 Spannung in Volt für Phase 1, float, Punkt als Trenner
    // openWB/set/evu/VPhase2 Spannung in Volt für Phase 2, float, Punkt als Trenner
    // openWB/set/evu/VPhase3 Spannung in Volt für Phase 3, float, Punkt als Trenner
    // openWB/set/evu/HzFrequenz oder openWB/set/evu/Hz Netzfrequenz in Hz, float, Punkt als Trenner                      -> done
    // openWB/set/evu/WPhase1 Leistung in Watt für Phase 1, float, Punkt als Trenner, positiv Bezug, negativ Einspeisung
    // openWB/set/evu/WPhase2 Leistung in Watt für Phase 2, float, Punkt als Trenner, positiv Bezug, negativ Einspeisung
    // openWB/set/evu/WPhase3 Leistung in Watt für Phase 3, float, Punkt als Trenner, positiv Bezug, negativ Einspeisung
    // openWB/set/evu/PfPhase1 Powerfaktor für Phase 1, float, Punkt als Trenner, positiv Bezug, negativ Einspeisung
    // openWB/set/evu/PfPhase2 Powerfaktor für Phase 2, float, Punkt als Trenner, positiv Bezug, negativ Einspeisung
    // openWB/set/evu/PfPhase3 Powerfaktor für Phase 3, float, Punkt als Trenner, positiv Bezug, negativ Einspeisung
    const std::string mosq_grid_power_str("openWB/set/evu/W");
    std::string grid_power = mTree.get<std::string>("ENERGY.GUI_GRID_POW"); // fl
    ConversionResultOpt grid_power_cr = Conversion::Convert(grid_power);
    if (grid_power_cr.is_initialized())
    {
      auto grid_pow = boost::get<float>(inverter_power_cr.get());
      mPublisher.publishInt(mosq_grid_power_str, static_cast<int>(grid_pow));
    }

    const std::string mosq_freq_str("openWB/set/evu/HzFrequenz");
    std::string frequency = mTree.get<std::string>("PM1OBJ1.FREQ"); // fl
    ConversionResultOpt frequency_cr = Conversion::Convert(frequency);
    if (frequency_cr.is_initialized())
    {
      auto freq = boost::get<float>(frequency_cr.get());
      mPublisher.publishFloat(mosq_freq_str, freq);
    }

    boost::property_tree::ptree power_vector_node = mTree.get_child("PM1OBJ1.P_AC"); // values not needed but signs for currents // fl
    const std::string mosq_grid_pow_a_str("openWB/set/evu/WPhase1");
    const std::string mosq_grid_pow_b_str("openWB/set/evu/WPhase2");
    const std::string mosq_grid_pow_c_str("openWB/set/evu/WPhase2");
    for (const auto& vec_entry : power_vector_node) {
      std::string name = vec_entry.first;  // Access vector name (if applicable)

      // Access vector data (assuming data is an array of doubles)
      // std::vector<std::string> values = vec_entry.second.get<std::vector<std::string> >("data");

      // Process vector data
      // ...
    }

    boost::property_tree::ptree voltage_vector_node = mTree.get_child("PM1OBJ1.U_AC"); // fl
    const std::string mosq_grid_volt_a_str("openWB/set/evu/VPhase1");
    const std::string mosq_grid_volt_b_str("openWB/set/evu/VPhase2");
    const std::string mosq_grid_volt_c_str("openWB/set/evu/VPhase3");

    // std::string grid_cur = mTree.get<std::string>("PM1OBJ1.I_AC");
    // std::vector<std::string> grid_cur_vec = mTree.get<std::vector<std::string>("PM1OBJ1.I_AC");
    boost::property_tree::ptree current_vector_node = mTree.get_child("PM1OBJ1.I_AC"); // fl
    const std::string mosq_grid_cur_a_str("openWB/set/evu/APhase1"); // todo take sign from power phase a
    const std::string mosq_grid_cur_b_str("openWB/set/evu/APhase2"); // todo take sign from power phase b
    const std::string mosq_grid_cur_c_str("openWB/set/evu/APhase3"); // todo take sign from power phase c

    // std::string house_power = mTree.get<std::string>("ENERGY.GUI_HOUSE_POW"); // fl

    // openWB/set/houseBattery/W Speicherleistung in Watt, int, positiv Ladung, negativ Entladung   -> done
    // openWB/set/houseBattery/WhImported Geladene Energie in Wh, float, nur positiv
    // openWB/set/houseBattery/WhExported Entladene Energie in Wh, float, nur positiv
    // openWB/set/houseBattery/%Soc Ladestand des Speichers, int, 0-100                             -> done

    const std::string mosq_bat_power_str("openWB/set/houseBattery/W");
    std::string bat_power = mTree.get<std::string>("ENERGY.GUI_BAT_DATA_POWER"); // fl
    ConversionResultOpt bat_power_cr = Conversion::Convert(bat_power);
    if (bat_power_cr.is_initialized())
    {
      auto bat_pow = boost::get<float>(bat_power_cr.get());
      mPublisher.publishInt(mosq_bat_power_str, static_cast<int>(bat_pow));
    }

    const std::string mosq_bat_soc_str("openWB/set/houseBattery/%Soc");
    std::string battery_soc = mTree.get<std::string>("ENERGY.GUI_BAT_DATA_FUEL_CHARGE"); // fl
    ConversionResultOpt battery_soc_cr = Conversion::Convert(battery_soc);
    if (battery_soc_cr.is_initialized())
    {
      auto bat_soc = boost::get<float>(battery_soc_cr.get());
      // std::cout << "SOC: " << bat_soc << std::endl;
      // std::cout << "SOC (str): " << std::to_string(bat_soc) << "\n";
      mPublisher.publishInt(mosq_bat_soc_str, static_cast<int>(bat_soc));
    }


  } catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
  }
}

// clang-format on