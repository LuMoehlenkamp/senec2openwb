#pragma once

#include "dataAcquisitionDefines.hh"

#include <cstring>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/optional.hpp>

namespace S2O {
const std::string ERROR = "error";
// clang-format off
class Conversion
{
public:
  static std::string floatToString(float value, int precision)
  {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(precision) << value;
    return ss.str();
  }

  static std::string doubleToString(double value, int precision)
  {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(precision) << value;
    return ss.str();
  }

  static void ConvertToString(const std::string &inString,
                              std::string &outString, bool invert = false,
                              int decimals = 1)
  {
    try {
      split_vector_type SplitVec;
      boost::split(SplitVec, inString, boost::is_any_of("_"),
                   boost::token_compress_on);
      std::stringstream ss;
      ss << std::hex << SplitVec[SplitVec.size() - 1];
      long value;
      ss >> value;
      if (SplitVec[0] == "fl") {
        float f_value;
        std::memcpy(&f_value, &value, sizeof(float));
        if (invert) {
          f_value *= -1.f;
        }
        outString = floatToString(f_value, decimals);
      } else if (SplitVec[0] == "u8" || SplitVec[0] == "u3") {
        if (value > static_cast<long>(UINT_MAX)) {
          throw std::out_of_range("Unsigned value overflows uint");
        }
        outString = std::to_string(static_cast<unsigned>(value));
      } else if (SplitVec[0] == "i3") {
        if (value > static_cast<long>(INT_MAX)) {
          throw std::out_of_range("Unsigned value overflows int");
        }
        if (invert) {
          value *= -1;
        }
        outString = std::to_string(value);
      }
    } catch (const std::exception &e) {
      std::cerr << e.what() << '\n';
    }
  }

  static void ConvertToFloatVal(const std::string &inString, float &outValue)
  {
    float value(0.0f);
    try {
      value = std::stof(inString);
    } catch (const std::invalid_argument &e) {
      std::cerr << "Ungültiger String für die Konvertierung: " << e.what()
                << '\n';
      outValue = 0.0f;
    }
    outValue = value;
  }
};

} // namespace S2O