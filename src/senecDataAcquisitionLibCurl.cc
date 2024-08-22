#include "senecDataAcquisitionLibCurl.hh"

#include <boost/property_tree/json_parser.hpp>
#include <chrono>
#include <iostream>

using namespace S2O;

// clang-format off
SenecDataAcquisitionLibCurl::SenecDataAcquisitionLibCurl(
    boost::asio::io_context &ioContext, unsigned TimerDuration, long TimeoutDuration_ms, long ConnectTimeoutDuration_ms)
    : mrIoContext(ioContext)
    , mTimerDuration(TimerDuration)
    , mTimeoutDuration_ms(TimeoutDuration_ms)
    , mConnectTimeoutDuration_ms(ConnectTimeoutDuration_ms)
    , mTimer(ioContext, std::chrono::seconds(INITIAL_TIMER_DURATION))
    , mPublisher() {
  Init();
  mTimer.async_wait(boost::bind(&SenecDataAcquisitionLibCurl::Acquire, this));
}

SenecDataAcquisitionLibCurl::~SenecDataAcquisitionLibCurl() {
  if (mCurl)
    curl_easy_cleanup(mCurl);
}

void SenecDataAcquisitionLibCurl::Init() {
  curl_global_init(CURL_GLOBAL_SSL);
}

void SenecDataAcquisitionLibCurl::Acquire() {
  mCurl = curl_easy_init();
  if (!mCurl) {
    // std::cout << "Curl not initialized" << '\n';
    setTimerDuration();
    return;
  }
  // std::cout << "attempting to acquire data" << '\n';
  CURLcode res;
  std::string response;
  curl_easy_setopt(mCurl, CURLOPT_URL, mUrl.c_str());
  curl_easy_setopt(mCurl, CURLOPT_POSTFIELDS, mPostData.c_str());
  curl_easy_setopt(mCurl, CURLOPT_POST, 1L);
  // curl_easy_setopt(mCurl, CURLOPT_VERBOSE, 1L);
  curl_easy_setopt(mCurl, CURLOPT_SSL_VERIFYPEER, false);
  curl_easy_setopt(mCurl, CURLOPT_WRITEFUNCTION, WriteCallback);
  curl_easy_setopt(mCurl, CURLOPT_WRITEDATA, &response);
  curl_easy_setopt(mCurl, CURLOPT_TIMEOUT_MS, mTimeoutDuration_ms);
  curl_easy_setopt(mCurl, CURLOPT_CONNECTTIMEOUT_MS, mConnectTimeoutDuration_ms);
  try {
    res = curl_easy_perform(mCurl);
  } catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
  }
  catch (...) {
    std::cerr << "unknown exception caught" << '\n';
  }

  if (res == CURLE_OK) {
    ParseResponse(response);
    if (mTree.empty()) {
      return;
    }
    ProcessData();
  }

  try {
    curl_easy_cleanup(mCurl);
  } catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
  }
  catch (...) {
    std::cerr << "unknown exception caught" << '\n';
  }
  setTimerDuration();
  return;
}

void SenecDataAcquisitionLibCurl::setTimerDuration() {
  mTimer.expires_after(std::chrono::seconds(mTimerDuration));
  mTimer.async_wait(boost::bind(&SenecDataAcquisitionLibCurl::Acquire, this));
}

size_t SenecDataAcquisitionLibCurl::WriteCallback(void *contents, size_t size,
                                                  size_t nmemb,
                                                  std::string *buffer) {
  // std::cout << "Write-cb called" << '\n';
  buffer->append((char *)contents, size * nmemb);
  return size * nmemb;
}

void SenecDataAcquisitionLibCurl::ParseResponse(const std::string &response) {
  mTree.clear();
  std::istringstream json_stream(response);
  try {
    boost::property_tree::read_json(json_stream, mTree);
  }
  catch(const boost::property_tree::json_parser_error& e) {
    std::cerr << "Exception: " << e.filename() << ": " << e.message() << '\n';
  }
  catch (const std::exception& e) {
    std::cerr << "Exception: " << e.what() << '\n';
  }
  catch (...) {
    std::cerr << "Exception of unknown type" << '\n';
  }

}

void SenecDataAcquisitionLibCurl::ProcessData() {
  try {
    std::string utc_offset = mTree.get<std::string>("RTC.UTC_OFFSET");
    ConversionResultOpt utc_offset_cr = Conversion::Convert(utc_offset);

    std::string time = mTree.get<std::string>("RTC.WEB_TIME");
    ConversionResultOpt time_cr = Conversion::Convert(time);

    // if (time_cr.is_initialized() && utc_offset_cr.is_initialized())
    // {
    //   auto utc_offset = boost::get<int>(utc_offset_cr.get());
    //   std::chrono::minutes offset_minutes(utc_offset);
    //   std::chrono::seconds offset_seconds(offset_minutes);

    // auto timestamp = boost::get<unsigned>(time_cr.get());
    // std::time_t time_s_epoch = static_cast<std::time_t>(timestamp);
    // std::stringstream ss;
    // ss << std::put_time(std::localtime(&time_s_epoch), "%F %T.\n");
    // mPublisher.publishTime(ss.str()); // todo: refactor this method
    // }

    // openWB/set/pv/1/W PV-Erzeugungsleistung in Watt, int, positiv
    const std::string topic_inv_power_str("openWB/set/pv/1/W");
    const std::string senec_inv_power_str("ENERGY.GUI_INVERTER_POWER");
    std::string inv_power_raw_str = mTree.get<std::string>(senec_inv_power_str);
    std::string inv_power_pub_str;
    Conversion::ConvertToString(inv_power_raw_str, inv_power_pub_str, false, false);
    mPublisher.publishStr(topic_inv_power_str, inv_power_pub_str);

    // openWB/set/pv/1/WhCounter Erzeugte Energie in Wh, float, nur positiv
    const std::string topic_inv_energy_str("openWB/set/pv/1/WhCounter");
    float inverter_power(std::stof(inv_power_pub_str));
    mInverterExportedEnergy.Integrate(std::abs(inverter_power));
    mPublisher.publishStr(topic_inv_energy_str, mInverterExportedEnergy.getIntegratedValueAsStr());

    // openWB/set/evu/W Bezugsleistung in Watt, int, positiv Bezug, negativ Einspeisung                                   -> done, float->int
    const std::string topic_grid_power_str("openWB/set/evu/W");
    const std::string senec_grid_power_str("ENERGY.GUI_GRID_POW");
    std::string grid_power_raw_str = mTree.get<std::string>(senec_grid_power_str);
    std::string grid_power_pub_str;
    Conversion::ConvertToString(grid_power_raw_str, grid_power_pub_str);
    mPublisher.publishStr(topic_grid_power_str, grid_power_pub_str);

    // openWB/set/evu/WhImported Bezogene Energie in Wh, float, Punkt als Trenner, nur positiv
    // openWB/set/evu/WhExported Eingespeiste Energie in Wh, float, Punkt als Trenner, nur positiv
    const std::string topic_grid_imported_energy_str("openWB/set/evu/WhImported");
    const std::string topic_grid_exported_energy_str("openWB/set/evu/WhExported");
    float grid_power(std::stof(grid_power_pub_str));
    std::signbit(grid_power) ? mGridExportedEnergy.Integrate(std::abs(grid_power)) : mGridImportedEnergy.Integrate(std::abs(grid_power));
    mPublisher.publishStr(topic_grid_imported_energy_str, mGridImportedEnergy.getIntegratedValueAsStr());
    mPublisher.publishStr(topic_grid_exported_energy_str, mGridExportedEnergy.getIntegratedValueAsStr());

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

    // openWB/set/houseBattery/W Speicherleistung in Watt, int, positiv Ladung, negativ Entladung   -> done
    const std::string topic_bat_power_str("openWB/set/houseBattery/W");
    const std::string senec_bat_power_str("ENERGY.GUI_BAT_DATA_POWER");
    std::string bat_power_raw_str = mTree.get<std::string>(senec_bat_power_str);
    std::string bat_power_pub_str;
    Conversion::ConvertToString(bat_power_raw_str, bat_power_pub_str);
    mPublisher.publishStr(topic_bat_power_str, bat_power_pub_str);

    // openWB/set/houseBattery/WhImported Geladene Energie in Wh, float, nur positiv
    // openWB/set/houseBattery/WhExported Entladene Energie in Wh, float, nur positiv
    const std::string topic_bat_imported_energy_str("openWB/set/houseBattery/WhImported");
    const std::string topic_bat_exported_energy_str("openWB/set/houseBattery/WhExported");
    float bat_power(std::stof(bat_power_pub_str));
    std::signbit(bat_power) ? mBatteryExportedEnergy.Integrate(std::abs(bat_power)) : mBatteryImportedEnergy.Integrate(std::abs(bat_power));
    mPublisher.publishStr(topic_bat_imported_energy_str, mBatteryImportedEnergy.getIntegratedValueAsStr());
    mPublisher.publishStr(topic_bat_exported_energy_str, mBatteryExportedEnergy.getIntegratedValueAsStr());

    // openWB/set/houseBattery/%Soc Ladestand des Speichers, int, 0-100
    const std::string topic_bat_soc_str("openWB/set/houseBattery/%Soc");
    const std::string senec_bat_soc_str("ENERGY.GUI_BAT_DATA_FUEL_CHARGE");
    std::string bat_soc_raw_str = mTree.get<std::string>(senec_bat_soc_str);
    std::string bat_soc_pub_str;
    Conversion::ConvertToString(bat_soc_raw_str, bat_soc_pub_str);
    mPublisher.publishStr(topic_bat_soc_str, bat_soc_pub_str);

  }
  catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
  }
  catch (...) {
    std::cerr << "unknown exception caught" << '\n';
  }
}
