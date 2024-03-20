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
class Conversion {
public:
  static float ConvertToFloat(const std::string &inString) {
    split_vector_type SplitVec;
    uint x;
    std::stringstream ss;

    boost::split(SplitVec, inString, boost::is_any_of("_"),
                 boost::token_compress_on);
    ss << std::hex << SplitVec[SplitVec.size() - 1];
    ss >> x;

    return reinterpret_cast<float &>(x);
  }

  static uint32_t ConvertToUint(const std::string &inString);

  static ConversionResultOpt Convert(const std::string &inString) {
    split_vector_type SplitVec;
    boost::split(SplitVec, inString, boost::is_any_of("_"),
                 boost::token_compress_on);
    std::stringstream ss;
    ConversionResult return_val;
    ss << std::hex << SplitVec[SplitVec.size() - 1];
    ulong value;
    ss >> value;
    if (SplitVec[0] == "fl") {
      return_val = reinterpret_cast<float &>(value);
      return return_val;
    } else if (SplitVec[0] == "u8" || SplitVec[0] == "u3") {
      if (value > static_cast<unsigned long long>(UINT_MAX)) {
        throw std::out_of_range("Unsigned value overflows uint");
      }
      return_val = static_cast<uint>(value);
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
                              bool keepDecimals = false) {
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
        if (value > static_cast<unsigned long long>(UINT_MAX)) {
          throw std::out_of_range("Unsigned value overflows uint");
        }
        outString = std::to_string(static_cast<uint>(value));
      } else if (SplitVec[0] == "i3") {
        if (value > static_cast<unsigned long long>(INT_MAX)) {
          throw std::out_of_range("Unsigned value overflows int");
        }
        outString = std::to_string(static_cast<int>(value));
      }
    } catch (const std::exception &e) {
      std::cerr << e.what() << '\n';
    }
  }

  // static ConversionResultOpt Convert(const std::string &inString) {
  //   std::stringstream ss(inString);
  //   std::string prefix;
  //   std::string value_string;
  //   constexpr char separator('_');
  //   constexpr char prefix_fl[]("fl");
  //   constexpr char prefix_i3[]("i3");
  //   constexpr char prefix_u3[]("u3");
  //   constexpr char prefix_u8[]("u8");
  //   ConversionResultOpt return_val;

  //   std::getline(ss, prefix, separator);
  //   // std::getline(ss, value_string);

  //   uint x;
  //   ss >> x;
  //   if (prefix == prefix_fl) {
  //     float value(0.0f);
  //     ss >> std::hex >> value;
  //     return_val = value;

  //   } else if (prefix == prefix_i3) {
  //     int value(0);
  //     ss >> std::hex >> value;
  //     return_val = value;

  //   } else if (prefix == prefix_u3 || prefix == prefix_u8) {
  //     unsigned long long value;
  //     ss >> std::hex >> value;
  //     if (value > static_cast<unsigned long long>(INT_MAX)) {
  //       throw std::out_of_range("Unsigned value overflows int");
  //     }
  //     return_val = static_cast<int>(value);
  //   }
  //   return return_val;
  // }
};

} // namespace S2O