find_package(Boost REQUIRED COMPONENTS
                              # filesystem
                              # program_options
                              regex
                              serialization
                              system
                              thread
                              )

find_package(PahoMqttCpp REQUIRED)
find_package(CURL REQUIRED)

add_definitions(-DBOOST_BIND_GLOBAL_PLACEHOLDERS)

# Add module support
# foreach(module ${CXX_MODULES})
#     add_custom_command(
#         OUTPUT ${module}.o
#         COMMAND ${CMAKE_CXX_COMPILER} ${CMAKE_CXX_FLAGS} -std=c++20 -c ${module} -o ${module}.o
#         DEPENDS ${module}
#     )
#     list(APPEND MODULE_OBJECTS ${module}.o)
# endforeach()

add_executable(${PROJECT_NAME_STR} main.cc configManager.cc senecDataAcquisitionLibCurl.cc mqttPublisher.cc integrator.cc)
# target_sources(${PROJECT_NAME_STR} PUBLIC FILE_SET modules TYPE CXX_MODULES FILES ${MODULE_FILES})

target_link_libraries(${PROJECT_NAME_STR} PUBLIC
                      ${Boost_LIBRARIES}
                      ${Boost_LOG_LIBRARY}
                      PahoMqttCpp::paho-mqttpp3-static
                      ${LIBS_SYSTEM}
                      stdc++
                      stdc++fs
                      ssl
                      crypto
                      CURL::libcurl)