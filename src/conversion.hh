#pragma once

#include "dataAcquisitionDefines.hh"

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
  static float ConvertToFloat(const std::string &inString)
  {
    split_vector_type SplitVec;
    unsigned x;
    std::stringstream ss;

    boost::split(SplitVec, inString, boost::is_any_of("_"),
                 boost::token_compress_on);
    ss << std::hex << SplitVec[SplitVec.size() - 1];
    ss >> x;
    auto ret = reinterpret_cast<float *>(&x);
    return *ret;
  }

  static uint32_t ConvertToUint(const std::string &inString);

  static ConversionResultOpt Convert(const std::string &inString)
  {
    split_vector_type SplitVec;
    boost::split(SplitVec, inString, boost::is_any_of("_"),
                 boost::token_compress_on);
    std::stringstream ss;
    ConversionResult return_val;
    ss << std::hex << SplitVec[SplitVec.size() - 1];
    unsigned long value;
    ss >> value;
    if (SplitVec[0] == "fl") {
      return_val = reinterpret_cast<float &>(value);
      return return_val;
    } else if (SplitVec[0] == "u8" || SplitVec[0] == "u3") {
      if (value > static_cast<unsigned long long>(UINT_MAX)) {
        throw std::out_of_range("Unsigned value overflows uint");
      }
      return_val = static_cast<unsigned>(value);
      return return_val;
    } else if (SplitVec[0] == "i3") {
      if (value > static_cast<unsigned long long>(INT_MAX)) {
        throw std::out_of_range("Unsigned value overflows int");
      }
      return_val = static_cast<int>(value);
      return return_val;
    }
    return boost::none;
  }

  static void ConvertToString(const std::string &inString,
                              std::string &outString, bool invert = false,
                              bool keepDecimals = false)
  {
    try {
      split_vector_type SplitVec;
      boost::split(SplitVec, inString, boost::is_any_of("_"),
                   boost::token_compress_on);
      std::stringstream ss;
      ConversionResult return_val;
      ss << std::hex << SplitVec[SplitVec.size() - 1];
      long value;
      ss >> value;
      if (invert) {
        value *= -1L;
      }
      if (SplitVec[0] == "fl") {
        float f_value = reinterpret_cast<float &>(value);
        if (keepDecimals)
          outString = std::to_string(f_value);
        else
          outString = std::to_string(static_cast<long>(f_value));
      } else if (SplitVec[0] == "u8" || SplitVec[0] == "u3") {
        if (value > static_cast<long>(UINT_MAX)) {
          throw std::out_of_range("Unsigned value overflows uint");
        }
        outString = std::to_string(static_cast<unsigned>(value));
      } else if (SplitVec[0] == "i3") {
        if (value > static_cast<long>(INT_MAX)) {
          throw std::out_of_range("Unsigned value overflows int");
        }
        outString = std::to_string(static_cast<long>(value));
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
                << std::endl;
      outValue = 0.0f;
      return;
    }
    outValue = value;
  }
};

} // namespace S2O