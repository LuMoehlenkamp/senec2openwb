#pragma once

#include <boost/optional.hpp>
#include <chrono>
#include <string>

namespace S2O {

class Integrator {
  static const int DECIMALS;

public:
  Integrator() = default;
  void Integrate(const float &value);
  bool DayChangeOccured(
      const std::chrono::time_point<std::chrono::system_clock> &time_now);
  void Reset();
  float getIntegratedValue() { return integrated_value; }
  std::string getIntegratedValueAsStr(int decimals = DECIMALS) const;

private:
  boost::optional<std::chrono::time_point<std::chrono::system_clock>>
      time_now_opt;
  boost::optional<std::chrono::time_point<std::chrono::system_clock>>
      time_last_opt;
  float integrated_value = 0.0f;
  int last_day = 0;
};
} // namespace S2O