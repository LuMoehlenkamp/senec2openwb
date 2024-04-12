find_package(Boost REQUIRED COMPONENTS
                              filesystem
                              # log_setup
                              # log
                              program_options
                              regex
                              serialization
                              system
                              thread
                              )

find_package(PahoMqttCpp REQUIRED)

add_definitions(-DBOOST_BIND_GLOBAL_PLACEHOLDERS)

add_executable(${PROJECT_NAME_STR} main.cc configManager.cc senecDataAcquisition.cc mqttPublisher.cc integrator.cc)

target_link_libraries(${PROJECT_NAME_STR} PUBLIC
                      ${Boost_LIBRARIES}
                      ${Boost_LOG_LIBRARY}
                      PahoMqttCpp::paho-mqttpp3-static
                      ${LIBS_SYSTEM}
                      stdc++fs
                      ssl
                      crypto)