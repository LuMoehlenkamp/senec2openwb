#pragma once

#include <string>
#include <vector>

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/variant.hpp>

namespace S2O {

const unsigned int HTTP_OK_STATUS = 200;
const unsigned INITIAL_TIMER_DURATION = 1;
typedef std::vector<std::string> split_vector_type;
typedef boost::property_tree::basic_ptree<std::string, std::string> ptree;
} // namespace S2O