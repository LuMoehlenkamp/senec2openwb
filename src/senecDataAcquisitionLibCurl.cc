#include "senecDataAcquisitionLibCurl.hh"

#include <boost/property_tree/json_parser.hpp>
#include <chrono>
#include <iostream>

using namespace S2O;

// clang-format off
SenecDataAcquisitionLibCurl::SenecDataAcquisitionLibCurl(
    boost::asio::io_context &ioContext, unsigned TimerDuration, long TimeoutDuration_ms, long ConnectTimeoutDuration_ms)
    : mrIoContext(ioContext)
    , mTimerDuration_ms(TimerDuration)
    , mTimeoutDuration_ms(TimeoutDuration_ms)
    , mConnectTimeoutDuration_ms(ConnectTimeoutDuration_ms)
    , mTimer(ioContext, std::chrono::seconds(INITIAL_TIMER_DURATION))
    , mPublisher()
{
  Init();
  mTimer.async_wait(boost::bind(&SenecDataAcquisitionLibCurl::Acquire, this));
}

SenecDataAcquisitionLibCurl::~SenecDataAcquisitionLibCurl()
{
  if (mCurl)
    curl_easy_cleanup(mCurl);
}

void SenecDataAcquisitionLibCurl::Init()
{
  curl_global_init(CURL_GLOBAL_SSL);
}

void SenecDataAcquisitionLibCurl::Acquire()
{
  setTimerDuration();
  mCurl = curl_easy_init();
  if (!mCurl) {
    std::cerr << "Curl not initialized" << '\n';
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
    return;
  }
  catch (...) {
    std::cerr << "unknown exception caught" << '\n';
    return;
  }

  if (res == CURLE_OK) {
    try {
      ParseResponse(response);
    }
    catch(const boost::property_tree::json_parser_error& e) {
      std::cerr << "JSON parser exception while trying to parse response: " << e.filename() << ": " << e.message() << '\n';
      return;
    }
    catch (const std::exception& e) {
      std::cerr << "std::exception exception while trying to parse response: " << e.what() << '\n';
      return;
    }
    catch (...) {
      std::cerr << "exception of unknown type while trying to parse response" << '\n';
      return;
    }

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
  return;
}

void SenecDataAcquisitionLibCurl::setTimerDuration()
{
  mTimer.expires_after(std::chrono::milliseconds(mTimerDuration_ms));
  mTimer.async_wait(boost::bind(&SenecDataAcquisitionLibCurl::Acquire, this));
}

size_t SenecDataAcquisitionLibCurl::WriteCallback(void *contents, size_t size,
                                                  size_t nmemb,
                                                  std::string *buffer)
{
  // buffer->append((char *)contents, size * nmemb);
  buffer->append(static_cast<char *>(contents), size * nmemb);
  return size * nmemb;
}

void SenecDataAcquisitionLibCurl::ParseResponse(const std::string &response)
{
  mTree.clear();
  std::istringstream json_stream(response);
  boost::property_tree::read_json(json_stream, mTree);
}

void SenecDataAcquisitionLibCurl::ProcessData()
{
  try {
    // ProcessTimeInformation();
    ProcessInverterData();
    ProcessGridData();
    ProcessBatteryData();
  }
  catch (const std::exception &e) {
    std::cerr << "std::exception while processing data: " << e.what() << '\n';
  }
  catch (...) {
    std::cerr << "unknown exception caught while processing data." << '\n';
  }
}

void S2O::SenecDataAcquisitionLibCurl::ProcessTimeInformation()
{
  auto utc_offset_str = mTree.get<std::string>(mTreeElemRtcOffset);
  ConversionResultOpt utc_offset_cr = Conversion::Convert(utc_offset_str);

  std::string time = mTree.get<std::string>(mTreeElemWebTime);
  ConversionResultOpt time_cr = Conversion::Convert(time);

  if (time_cr.is_initialized() && utc_offset_cr.is_initialized())
  {
    auto utc_offset = boost::get<int>(utc_offset_cr.get());
    std::chrono::minutes offset_minutes(utc_offset);
    std::chrono::seconds offset_seconds(offset_minutes);

    auto timestamp = boost::get<unsigned>(time_cr.get());
    std::time_t time_s_epoch = static_cast<std::time_t>(timestamp);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_s_epoch), "%F %T.\n");
    mPublisher.publishTime(ss.str()); // todo: refactor this method
  }
}

void S2O::SenecDataAcquisitionLibCurl::ProcessInverterData()
{
  // openWB/set/pv/1/W PV-Erzeugungsleistung in Watt, int, positiv
  std::string inv_power_raw_str = mTree.get<std::string>(mTreeElemInvPower);
  std::string inv_power_pub_str;
  Conversion::ConvertToString(inv_power_raw_str, inv_power_pub_str, false, false);
  mPublisher.publishStr(mTopicInvPower, inv_power_pub_str);

  // openWB/set/pv/1/WhCounter Erzeugte Energie in Wh, float, nur positiv
  float inverter_power(std::stof(inv_power_pub_str));
  mInverterExportedEnergy.Integrate(std::abs(inverter_power));
  mPublisher.publishStr(mTopicInvEnergy, mInverterExportedEnergy.getIntegratedValueAsStr());
}

void S2O::SenecDataAcquisitionLibCurl::ProcessGridData()
{
  // openWB/set/evu/W Bezugsleistung in Watt, int, positiv Bezug, negativ Einspeisung
  std::string grid_power_raw_str = mTree.get<std::string>(mTreeElemGridPower);
  std::string grid_power_pub_str;
  Conversion::ConvertToString(grid_power_raw_str, grid_power_pub_str);
  mPublisher.publishStr(mTopicGridPower, grid_power_pub_str);

  // openWB/set/evu/WhImported Bezogene Energie in Wh, float, Punkt als Trenner, nur positiv
  // openWB/set/evu/WhExported Eingespeiste Energie in Wh, float, Punkt als Trenner, nur positiv
  float grid_power(std::stof(grid_power_pub_str));
  std::signbit(grid_power) ? mGridExportedEnergy.Integrate(std::abs(grid_power)) : mGridImportedEnergy.Integrate(std::abs(grid_power));
  mPublisher.publishStr(mTopicGridImportedEnergy, mGridImportedEnergy.getIntegratedValueAsStr());
  mPublisher.publishStr(mTopicGridExportedEnergy, mGridExportedEnergy.getIntegratedValueAsStr());

  // openWB/set/evu/HzFrequenz oder openWB/set/evu/Hz Netzfrequenz in Hz, float, Punkt als Trenner
  std::string frequency_raw_str = mTree.get<std::string>(mTreeElemFreq);
  std::string frequency_pub_str;
  Conversion::ConvertToString(frequency_raw_str, frequency_pub_str, false, true);
  mPublisher.publishStr(mTopicFrequency, frequency_pub_str);

  // openWB/set/evu/WPhase1 (2,3) Leistung in Watt für Phase 1 (2,3), float, Punkt als Trenner, positiv Bezug, negativ Einspeisung
  std::vector<std::string> powers_raw_str_vec;
  std::vector<float> powers_fl_vec;
  for (boost::property_tree::ptree::value_type &power : mTree.get_child(mTreeElemGridPowers))
  {
    powers_raw_str_vec.push_back(power.second.data());
  }

  auto power_vals_it(mPowerValues.begin());
  for (auto pow_topics_it = mTopicGridPowersVec.begin(), raw_it = powers_raw_str_vec.begin();
       raw_it != powers_raw_str_vec.end();
       ++pow_topics_it, ++raw_it, ++power_vals_it)
  {
    std::string power_pub_str;
    Conversion::ConvertToString(*raw_it, power_pub_str, false, true);
    powers_fl_vec.push_back(std::stof(power_pub_str));
    mPublisher.publishStr(*pow_topics_it, power_pub_str);
    Conversion::ConvertToFloatVal(power_pub_str, *power_vals_it);
  }

  // openWB/set/evu/VPhase1 (2,3) Spannung in Volt für Phase 1 (2,3), float, Punkt als Trenner
  std::vector<std::string> volts_raw_str_vec;
  for (boost::property_tree::ptree::value_type &voltage : mTree.get_child(mTreeElemVoltages))
  {
    volts_raw_str_vec.push_back(voltage.second.data());
  }

  auto volt_vals_it(mVoltageValues.begin());
  for (auto volt_topics_it = mTopicGridVoltagesVec.begin(), raw_it = volts_raw_str_vec.begin();
       raw_it != volts_raw_str_vec.end();
       ++volt_topics_it, ++raw_it, ++volt_vals_it)
  {
    std::string voltage_pub_str;
    Conversion::ConvertToString(*raw_it, voltage_pub_str, false, true);
    mPublisher.publishStr(*volt_topics_it, voltage_pub_str);
    Conversion::ConvertToFloatVal(voltage_pub_str, *volt_vals_it);
  }

  // openWB/set/evu/APhase1 (2,3) Strom in Ampere für Phase 1 (2,3), float, Punkt als Trenner, positiv Bezug, negativ Einspeisung
  std::vector<std::string> amps_raw_str_vec;
  for (boost::property_tree::ptree::value_type &current : mTree.get_child(mTreeElemCurrents))
  {
    amps_raw_str_vec.push_back(current.second.data());
  }

  auto amps_vals_it(mCurrentValues.begin());
  auto amps_topics_it = mTopicGridCurrentsVec.begin();
  auto raw_it = amps_raw_str_vec.begin();
  auto powers_it = powers_fl_vec.begin();
  for ( ;
       raw_it != amps_raw_str_vec.end();
       ++amps_topics_it, ++raw_it, ++powers_it, ++amps_vals_it)
  {
    std::string current_pub_str;
    Conversion::ConvertToString(*raw_it, current_pub_str, std::signbit(*powers_it), true);
    mPublisher.publishStr(*amps_topics_it, current_pub_str);
    Conversion::ConvertToFloatVal(current_pub_str, *amps_vals_it);
  }

  // openWB/set/evu/PfPhase1 (2,3) Powerfaktor für Phase 1 (2,3), float, Punkt als Trenner, positiv Bezug, negativ Einspeisung
  auto pf_topics_it = mTopicGridPowerFactorVec.begin();
  auto volt_value_it = mVoltageValues.begin();
  auto amps_value_it = mCurrentValues.begin();
  auto pow_value_it = mPowerValues.begin();
  for (; pf_topics_it != mTopicGridPowerFactorVec.end(); ++pf_topics_it, ++volt_value_it, ++amps_value_it, ++pow_value_it)
  {
    float apparent_power(*volt_vals_it * *amps_value_it);
    float power_factor(*pow_value_it / apparent_power);
    mPublisher.publishStr(*pf_topics_it, std::to_string(power_factor));
  }
}

void S2O::SenecDataAcquisitionLibCurl::ProcessBatteryData()
{
  // openWB/set/houseBattery/W Speicherleistung in Watt, int, positiv Ladung, negativ Entladung   -> done
  std::string bat_power_raw_str = mTree.get<std::string>(mTreeElemBatteryPower);
  std::string bat_power_pub_str;
  Conversion::ConvertToString(bat_power_raw_str, bat_power_pub_str);
  mPublisher.publishStr(mTopicBatteryPower, bat_power_pub_str);

  // openWB/set/houseBattery/WhImported Geladene Energie in Wh, float, nur positiv
  // openWB/set/houseBattery/WhExported Entladene Energie in Wh, float, nur positiv
  float bat_power(std::stof(bat_power_pub_str));
  std::signbit(bat_power) ? mBatteryExportedEnergy.Integrate(std::abs(bat_power)) : mBatteryImportedEnergy.Integrate(std::abs(bat_power));
  mPublisher.publishStr(mTopicBatteryImportedEnergy, mBatteryImportedEnergy.getIntegratedValueAsStr());
  mPublisher.publishStr(mTopicBatteryExportedEnergy, mBatteryExportedEnergy.getIntegratedValueAsStr());

  // openWB/set/houseBattery/%Soc Ladestand des Speichers, int, 0-100
  std::string bat_soc_raw_str = mTree.get<std::string>(mTreeElemBatterySoc);
  std::string bat_soc_pub_str;
  Conversion::ConvertToString(bat_soc_raw_str, bat_soc_pub_str);
  mPublisher.publishStr(mTopicBatterySoc, bat_soc_pub_str);
}
