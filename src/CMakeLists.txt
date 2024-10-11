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

# set(PROJECT_LIB ${PROJECT_LIB} ${Boost_LOG_LIBRARY} ${Boost_LOG_SETUP_LIBRARY})

# find_package(OpenSSL "3.0" REQUIRED)
# MESSAGE (STATUS "Found? ${OPENSSL_FOUND}")
# MESSAGE (STATUS "Root: ${OPENSSL_ROOT_DIR}")
# MESSAGE (STATUS "Incl: ${OPENSSL_INCLUDE_DIR}")
# MESSAGE (STATUS "Crypt: ${OPENSSL_CRYPTO_LIBRARY}")
find_package(PahoMqttCpp REQUIRED)

add_definitions(-DBOOST_BIND_GLOBAL_PLACEHOLDERS)

add_executable(${PROJECT_NAME_STR} main.cc configManager.cc senecDataAcquisitionLibCurl.cc mqttPublisher.cc integrator.cc)

target_link_libraries(${PROJECT_NAME_STR} PUBLIC
                      ${Boost_LIBRARIES}
                      ${Boost_LOG_LIBRARY}
                      PahoMqttCpp::paho-mqttpp3-static
                      ${LIBS_SYSTEM}
                      stdc++
                      stdc++fs
                      ssl
                      crypto
                      curl)