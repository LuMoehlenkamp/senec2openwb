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
typedef boost::variant<uint, int, float> ConversionResult;
typedef boost::optional<ConversionResult> ConversionResultOpt;
typedef boost::optional<float> float_opt;

const std::string GET = "GET";
const std::string POST = "POST";
const std::string BLANK = " ";
const std::string SENEC_PATH = "/lala.cgi";
const std::string HTTP_VERSION = "HTTP/1.1";
const std::string CRLF = "\r\n";
const std::string HOST = "Host:";
const std::string SENEC_IP = "192.168.178.40";
const std::string HTTPS = "https://";
const std::string USER_AGENT = "User-Agent:";
const std::string USER_AGENT_VALUE = "C/1.0";
const std::string CONTENT_TYPE = "Content-Type:";
const std::string CONTENT_TYPE_VALUE =
    "application/x-www-form-urlencoded; charset=UTF-8";
const std::string CONTENT_LENGTH = "Content-Length:";
const std::string ACCEPT = "Accept:";
const std::string ACCEPT_VALUE =
    "application/json, text/javascript, */*; q=0.01";
const std::string CONNECTION = "Connection:";
const std::string CONNECTION_KEEP_ALIVE_VALUE = "keep-alive";
const std::string CONNECTION_CLOSE_VALUE = "close";
const std::string ACCEPT_ENCODING = "Accept-Encoding:";
const std::string ACCEPT_ENCODING_VALUE = "gzip, deflate, br";
const std::string ACCEPT_LANGUAGE = "Accept-Language:";
const std::string ACCEPT_LANGUAGE_VALUE = "en-US,en;q=0.5";
const std::string X_REQUEST_WITH = "X-Requested-With:";
const std::string X_REQUEST_WITH_VALUE = "XMLHttpRequest";
const std::string ORIGIN = "Origin:";
const std::string ORIGIN_VALUE = "https://192.168.178.40";
const std::string REFERER = "Referer:";
const std::string REFERER_VALUE = "https://192.168.178.40";
const std::string SEC_FETCH_DEST = "Sec-Fetch-Dest:";
const std::string SEC_FETCH_DEST_VALUE = "empty";
const std::string SEC_FETCH_MODE = "Sec-Fetch-Mode:";
const std::string SEC_FETCH_MODE_VALUE = "cors";
const std::string SEC_FETCH_SITE = "Sec-Fetch-Site:";
const std::string SEC_FETCH_SITE_VALUE = "same-origin";

const std::string SENEC_REQUEST =
    "{\"PM1OBJ1\":{}, \"PM1OBJ2\":{}, \"BAT1OBJ1\":{}, \"STATISTIC\":{}, "
    "\"ENERGY\":{}}";

const std::string EOF_MESSAGE = "0\r\n\r\n";
} // namespace S2O