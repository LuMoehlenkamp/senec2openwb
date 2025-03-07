set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ".")
set(PROJECT_UNIT_TESTS_NAME ${PROJECT_NAME_STR}_unit_tests)

# Dependencies
find_package(Boost REQUIRED COMPONENTS
                      filesystem
                      # log_setup
                      # log
                      program_options
                      regex
                      serialization
                      system
                      thread
                      timer
                      unit_test_framework)

# Gather the header and source files
file(GLOB UNIT_TESTS_INC_FILES ${PROJECT_SOURCE_DIR}/test/*.hh)
file(GLOB UNIT_TESTS_SRC_FILES ${PROJECT_SOURCE_DIR}/test/*.cc)

# Include paths
set(UNIT_TESTS_INCLUDES ${PROJECT_INCLUDE_DIR}
        ${PROJECT_SOURCE_DIR}/src)
        # ${Boost_INCLUDE_DIR})

        # Assign the include directories
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${UNIT_TESTS_INCLUDES})

find_package(PahoMqttCpp REQUIRED)

# --- build test executables ---
add_definitions(-DBOOST_TEST_DYN_LINK)
add_definitions(-DBOOST_BIND_GLOBAL_PLACEHOLDERS)
add_executable(${PROJECT_UNIT_TESTS_NAME} ${UNIT_TESTS_SRC_FILES} ../src/configManager.cc ../src/integrator.cc)
# add_executable(${PROJECT_UNIT_TESTS_NAME} mainTest.cc powerControlStrategyExcessTest.cc ../src/configManager.cc ../src/powerControlStrategyExcess.cc ../src/senecDataAcquisition.cc ../src/solarDataAcquisition.cc ../src/oneWireSensorInterface.cc)
# add_executable(${PROJECT_UNIT_TESTS_NAME} mainTest.cc oneWireSensorInterfaceTest.cc ../src/oneWireSensorInterface.cc)
# set_target_properties(${PROJECT_UNIT_TESTS_NAME} PROPERTIES log_level message)
# find_library(WIRINGPI_LIBRARIES NAMES wiringPi PATHS /usr/local/include/wiringPi.h)

target_link_libraries(${PROJECT_UNIT_TESTS_NAME}
                      ${Boost_LIBRARIES}
                      ${Boost_LOG_LIBRARY}
                      PahoMqttCpp::paho-mqttpp3-static
                      ${LIBS_SYSTEM}
                      stdc++
                      stdc++fs
                      ssl
                      crypto
                      curl
                      )
enable_testing()
add_test(MainTest ${PROJECT_UNIT_TESTS_NAME})