#pragma once

#include <boost/optional.hpp>
#include <chrono>

namespace S2O {

class Integrator {
public:
  Integrator() = default;
  void Integrate(const float &value);
  bool DayChangeOccured(
      const std::chrono::time_point<std::chrono::system_clock> &time_now);
  void Reset();
  float getIntegratedValue() { return integrated_value; }

private:
  boost::optional<std::chrono::time_point<std::chrono::system_clock>>
      time_now_opt;
  boost::optional<std::chrono::time_point<std::chrono::system_clock>>
      time_last_opt;
  float integrated_value = 0.0f;
  int last_day = 0;
};
} // namespace S2O