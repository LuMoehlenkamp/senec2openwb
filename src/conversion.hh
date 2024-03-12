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
    ss << std::hex << SplitVec[SplitVec.size() - 1];
    uint x;
    ss >> x;
    ConversionResult return_val;
    if (SplitVec[0] == "fl") {
      return_val = reinterpret_cast<float &>(x);
      return return_val;
    }
    if (SplitVec[0] == "u8" || SplitVec[0] == "u3") {
      return_val = x;
      return return_val;
    }
    return boost::none;
  }
};

} // namespace S2O