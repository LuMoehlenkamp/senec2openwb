#include "integrator.hh"
#include "mqttPublisher.hh"

#include <iostream>
#include <istream>
#include <ostream>
#include <string>

#include <boost/algorithm/string.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/optional.hpp>
#include <boost/property_tree/ptree.hpp>

namespace S2O {

class SenecDataAcquisition {
public:
  SenecDataAcquisition(boost::asio::io_context &ioContext,
                       boost::asio::ssl::context &sslContext,
                       unsigned TimerDuration);
  SenecDataAcquisition(SenecDataAcquisition &&) = delete;
  SenecDataAcquisition(const SenecDataAcquisition &) = delete;

  void Acquire();

private:
  const unsigned short mPort = 80;
  boost::asio::ip::tcp::endpoint mEndpoint;
  boost::asio::io_context &mrIoContext;
  boost::asio::ssl::context &mrSslContext;
  boost::asio::ip::tcp::resolver mResolver;
  boost::asio::ip::tcp::socket mTcpSocket; // ToDo: remove
  boost::asio::ssl::stream<boost::asio::ip::tcp::socket> mSslSocket;
  // ptree mTree;
  unsigned mTimerDuration;
  boost::asio::steady_timer mTimer;
  boost::asio::streambuf mRequest;
  boost::asio::streambuf mResponse;

  mqttPublisher mPublisher;
  Integrator mInverterExportedEnergy;
  Integrator mGridImportedEnergy;
  Integrator mGridExportedEnergy;
  Integrator mBatteryImportedEnergy;
  Integrator mBatteryExportedEnergy;
};

} // namespace S2O