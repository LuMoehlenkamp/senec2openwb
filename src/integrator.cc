#include "integrator.hh"
#include <algorithm>
#include <ctime>
#include <iomanip>
#include <limits>
#include <sstream>

using namespace std::chrono_literals;
using namespace S2O;

const int Integrator::DECIMALS = 1;

void Integrator::Integrate(const float &value) {
  time_now_opt = std::chrono::system_clock::now();
  if (time_now_opt.is_initialized()) {
    if (DayChangeOccured(time_now_opt.get()))
      Reset();
    if (time_last_opt.is_initialized()) {
      auto delta = time_now_opt.get() - time_last_opt.get();
      constexpr auto ref(
          std::chrono::duration_cast<std::chrono::nanoseconds>(1h));

      auto ratio(static_cast<float>(delta.count()) /
                 static_cast<float>(ref.count()));

      integrated_value = std::clamp(integrated_value + ratio * value,
                                    std::numeric_limits<float>::lowest(),
                                    std::numeric_limits<float>::max());
    }
  }
  time_last_opt = time_now_opt;
}

bool Integrator::DayChangeOccured(
    const std::chrono::time_point<std::chrono::system_clock> &time_now) {
  auto now_time_t(std::chrono::system_clock::to_time_t(time_now));
  std::tm *time_info_now = std::localtime(&now_time_t);
  int current_day = time_info_now->tm_mday;

  bool return_val(false);
  if (current_day != last_day)
    return_val = true;
  last_day = current_day;

  return return_val;
}

void Integrator::Reset() { integrated_value = 0.00f; }

std::string Integrator::getIntegratedValueAsStr(int decimals) const {
  std::stringstream ss;
  ss << std::fixed << std::setprecision(decimals) << integrated_value;
  return ss.str();
}
