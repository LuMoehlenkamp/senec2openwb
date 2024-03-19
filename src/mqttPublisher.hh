#include <atomic>
#include <string>

#include "mqtt/async_client.h"

namespace S2O {

class callback : public virtual mqtt::callback {
public:
  void connection_lost(const std::string &cause) override {
    std::cout << "\nConnection lost" << '\n';
    if (!cause.empty())
      std::cout << "\tcause: " << cause << '\n';
  }

  void delivery_complete(mqtt::delivery_token_ptr tok) override {
    // std::cout << "\tDelivery complete for token: "
    //           << (tok ? tok->get_message_id() : -1) << '\n';
    ;
  }
};

class action_listener : public virtual mqtt::iaction_listener {
protected:
  void on_failure(const mqtt::token &tok) override {
    std::cout << "\tListener failure for token: " << tok.get_message_id()
              << '\n';
  }

  void on_success(const mqtt::token &tok) override {
    std::cout << "\tListener success for token: " << tok.get_message_id()
              << '\n';
  }
};

class delivery_action_listener : public action_listener {
  std::atomic<bool> done_;

  void on_failure(const mqtt::token &tok) override {
    action_listener::on_failure(tok);
    done_ = true;
  }

  void on_success(const mqtt::token &tok) override {
    action_listener::on_success(tok);
    done_ = true;
  }

public:
  delivery_action_listener() : done_(false) {}
  bool is_done() const { return done_; }
};

class mqttPublisher {
public:
  mqttPublisher();
  void publishTime(const std::string &timestr);
  void publishFloat(const std::string &topic, float value);

private:
  std::string mIpAddress;
  std::string mClientId;
  mqtt::async_client mClient;
  mqtt::connect_options mConnOpts;
  callback mCallback;
};

} // namespace S2O
