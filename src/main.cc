#include "configManager.hh"
#include "mqtt/async_client.h"
// #include "senecDataAcquisitionCurl.hh"
#include "senecDataAcquisitionLibCurl.hh"

#include <iostream>

#define MQTT5 // Comment to use MQTT version 3.1.1

namespace {
volatile std::sig_atomic_t gSignalStatus;
}

void SignalHandler(int signal) {
  gSignalStatus = signal;
  std::cout << '\n' << "Received signal: " << gSignalStatus << '\n';
  exit(1);
}

int main() {
  S2O::ConfigManager *p_config_manager(
      S2O::ConfigManager::GetInstance(S2O::ConfigManager::CONFIG_PATH));
  auto senec_update_time_opt = p_config_manager->GetSenecUpdateTime();
  auto senec_timeout_time_opt = p_config_manager->GetSenecTimeoutTime();
  auto senec_connect_timeout_time_opt =
      p_config_manager->GetSenecConnectTimeoutTime();
  if (!senec_update_time_opt.is_initialized() ||
      !senec_timeout_time_opt.is_initialized() ||
      !senec_connect_timeout_time_opt.is_initialized()) {
    std::cerr << "incomplete configuration. GoodBye!" << "\n";
    return -1;
  }
  try {
    boost::asio::io_context ioContext;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type>
        work = boost::asio::make_work_guard(ioContext);
    // clang-format off
    S2O::SenecDataAcquisitionLibCurl senec_da_lc(
        ioContext,
        senec_update_time_opt.get(),
        senec_timeout_time_opt.get(),
        senec_connect_timeout_time_opt.get());
    // clang-format on
    std::signal(SIGINT, ::SignalHandler);  // SIGINT 2
    std::signal(SIGTERM, ::SignalHandler); // SIGTERM 15
    ioContext.run();
  } catch (std::exception &e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }
  return 0;
}