#include "configManager.hh"
#include "mqtt/async_client.h"
#include "senecDataAcquisitionCurl.hh"

#include <iostream>

#define MQTT5 // Comment to use MQTT version 3.1.1

namespace {
volatile std::sig_atomic_t gSignalStatus;
}

void SignalHandler(int signal) {
  gSignalStatus = signal;
  std::cout << '\n' << "received " << gSignalStatus << '\n';
  exit(1);
}

int main(int argc, char *argv[]) {
  S2O::ConfigManager *p_config_manager(
      S2O::ConfigManager::GetInstance(S2O::ConfigManager::CONFIG_PATH));
  auto senec_update_time_opt = p_config_manager->GetSenecUpdateTime();
  try {
    boost::asio::io_context ioContext;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type>
        work = boost::asio::make_work_guard(ioContext);
    S2O::SenecDataAcquisitionCurl senec_cda(ioContext,
                                            senec_update_time_opt.get());
    std::signal(SIGINT, ::SignalHandler);  // SIGINT 2
    std::signal(SIGTERM, ::SignalHandler); // SIGTERM 15
    ioContext.run();
  } catch (std::exception &e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }
  return 0;
}