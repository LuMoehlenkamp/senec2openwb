#include "configManager.hh"
#include "mqtt/async_client.h"
#include "senecDataAcquisition.hh"
// #include "senecDataAcquisitionCurl.hh"

#include <iostream>

namespace asio = boost::asio;
namespace ssl = boost::asio::ssl;

#define MQTT5 // Comment to use MQTT version 3.1.1

int main(int argc, char *argv[]) {
  S2O::ConfigManager *p_config_manager(
      S2O::ConfigManager::GetInstance(S2O::ConfigManager::CONFIG_PATH));
  auto senec_update_time_opt = p_config_manager->GetSenecUpdateTime();
  if (!senec_update_time_opt.is_initialized()) {
    std::cout << "Param 'senecUpdateTime_sec' not set. Aborting." << '\n';
    exit(1);
  }
  unsigned senec_update_time;

  try {
    asio::io_context ioContext;
    ssl::context ctx{ssl::context::tlsv12_client};

    asio::executor_work_guard<asio::io_context::executor_type> work =
        asio::make_work_guard(ioContext);
    // S2O::SenecDataAcquisitionCurl senec_cda(ioContext,
    //                                         senec_update_time_opt.get());
    S2O::SenecDataAcquisition senec_da(ioContext, ctx,
                                       senec_update_time_opt.get());
    boost::asio::signal_set signals(ioContext, SIGINT, SIGTERM);
    signals.async_wait(
        [&ioContext](const boost::system::error_code &ec, int signal) {
          std::cout << '\n' << "received: " << signal << '\n';
          // Stop the io_context. This will cause run()
          // to return immediately, eventually destroying the
          // io_context and any remaining handlers in it.
          ioContext.stop();
        });
    ioContext.run();
  } catch (std::exception &e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }
  return EXIT_SUCCESS;
}