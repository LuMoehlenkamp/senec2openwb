#include "mqttPublisher.hh"

#include <iostream> // ToDo: rmove after testing
#include <sstream>

using namespace S2O;

const int QOS(0);
const auto TIMEOUT = std::chrono::seconds(10);

const std::string TOPIC{"hello"};
const char *LWT_PAYLOAD = "Last will and testament.";

mqttPublisher::mqttPublisher()
    : mIpAddress("mosquitto"), mClientId("myClient"),
      mClient(mIpAddress, mClientId)

{
  mClient.set_callback(mCallback);
  mConnOpts = mqtt::connect_options_builder()
                  .clean_session()
                  .will(mqtt::message(TOPIC, LWT_PAYLOAD, strlen(LWT_PAYLOAD),
                                      QOS, false))
                  .finalize();
  try {
    mqtt::token_ptr conntok = mClient.connect(mConnOpts);
    conntok->wait();
  } catch (const mqtt::exception &e) {
    std::cerr << e.what() << '\n';
    return;
  } catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
  }
}

void mqttPublisher::publishTime(const std::string &timestr) {
  mqtt::message_ptr pubmsg =
      mqtt::make_message("SENEC_TIME", timestr.c_str(), QOS, false);
  if (mClient.is_connected())
    mClient.publish(pubmsg)->wait_for(TIMEOUT);
}

void mqttPublisher::publishFloat(const std::string &topic, float value) {
  mqtt::message_ptr pubmsg =
      mqtt::make_message(topic, std::to_string(value).c_str(), QOS, false);
  if (mClient.is_connected())
    mClient.publish(pubmsg)->wait_for(TIMEOUT);
}

void mqttPublisher::publishInt(const std::string &topic, int value) {
  mqtt::message_ptr pubmsg =
      mqtt::make_message(topic, std::to_string(value).c_str(), QOS, false);
  if (mClient.is_connected())
    mClient.publish(pubmsg)->wait_for(TIMEOUT);
}

void mqttPublisher::publishStr(const std::string &topic,
                               const std::string &valueStr) {
  mqtt::message_ptr pubmsg =
      mqtt::make_message(topic, valueStr.c_str(), QOS, false);
  if (mClient.is_connected())
    mClient.publish(pubmsg)->wait_for(TIMEOUT);
}

void mqttPublisher::publishStrVec(const std::string &topic,
                                  const std::vector<std::string> &valueStrVec) {
  std::string serialized_vec;
  serialize(valueStrVec, serialized_vec);
  publishStr(topic, serialized_vec);
  std::cout << serialized_vec << '\n';
}

void mqttPublisher::serialize(const std::vector<std::string> &valueStrVec,
                              std::string resultStr) const {
  std::stringstream ss;
  bool first = true;
  for (const auto &str : valueStrVec) {
    if (first) {
      first = false;
    } else {
      ss << ",";
    }
    ss << str;
  }
  resultStr = ss.str();
}
