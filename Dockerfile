# uses a multi-stage build strategy

# senec2openwb build stage
# takes the base alpine image and enhances it with the build time dependencies
FROM --platform=$BUILDPLATFORM alpine:3.19.0 AS build
RUN uname -a
ARG TARGETARCH

RUN apk update && apk add --no-cache build-base cmake boost1.82-dev=1.82.0-r3 mosquitto-dev git curl curl-dev openssl openssl-dev g++-cross-embedded gcc-cross-embedded
# gcc-aarch64-none-elf g++-aarch64-none-elf g++-cross-embedded gcc-cross-embedded g++-cross-embedded gcc-aarch64-none-elf g++-aarch64-none-elf openssl openssl openssl-dev

WORKDIR /senec2openwb
RUN git clone --recurse-submodules https://github.com/eclipse/paho.mqtt.cpp.git

ARG TARGETPLATFORM

WORKDIR /senec2openwb/paho.mqtt.cpp
RUN cd /senec2openwb/paho.mqtt.cpp
RUN git checkout v1.4.0
RUN git submodule init
RUN git submodule update
RUN cmake -Bbuild -H. -DPAHO_WITH_MQTT_C=ON -DPAHO_BUILD_STATIC=ON -DPAHO_BUILD_DOCUMENTATION=OFF -DPAHO_BUILD_SAMPLES=OFF -DPAHO_WITH_SSL=OFF
RUN cmake --build build/ --target install --parallel 4
RUN ldconfig /etc/ld.so.conf.d

WORKDIR /senec2openwb
COPY src/ ./src/
COPY test/ ./test/
COPY CMakeLists.txt .
COPY toolchain.cmake .

WORKDIR /senec2openwb/build
RUN cmake -DCMAKE_BUILD_TYPE=Release .. && cmake --build . --parallel 4
# -DCMAKE_TOOLCHAIN_FILE=./toolchain.cmake
# senec2openwb run stage
FROM alpine:3.19.0
# FROM arm64v8/alpine:latest

RUN apk update && apk add --no-cache curl libstdc++ boost1.82-program_options=1.82.0-r3 boost1.82-filesystem=1.82.0-r3 boost1.82-log=1.82.0-r3 boost1.82-log_setup=1.82.0-r3
RUN addgroup -S ludger && adduser -S ludger -G ludger
USER ludger:ludger
COPY --chown=ludger:ludger --from=build ./senec2openwb/bin/senec2openwb /app/
RUN mkdir -p /app/dat
RUN chmod -R 755 /app/dat/
RUN mkdir -p /app/res
RUN chmod -R 755 /app/res/

WORKDIR /app
USER ludger:ludger
ENTRYPOINT [ "./senec2openwb" ]