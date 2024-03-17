# uses a multi-stage build strategy

# senec2openwb build stage
# takes the base alpine image and enhances it with the build time dependencies
FROM alpine:3.19.0 AS build

RUN apk update && apk add --no-cache build-base cmake boost1.82-dev=1.82.0-r3 mosquitto-dev git openssl-dev

WORKDIR /senec2openwb
RUN git clone --recurse-submodules https://github.com/eclipse/paho.mqtt.cpp.git

WORKDIR /senec2openwb/paho.mqtt.cpp
# ToDo: try avoiding this specific checkout
RUN git checkout ef021805c7aca42d72194d85bb2bdccdfe963e28
RUN cmake -Bbuild -H. -DPAHO_WITH_MQTT_C=ON -DPAHO_BUILD_STATIC=ON -DPAHO_BUILD_DOCUMENTATION=OFF -DPAHO_BUILD_SAMPLES=OFF
RUN cmake --build build/ --target install
RUN ldconfig /etc/ld.so.conf.d

WORKDIR /senec2openwb
COPY src/ ./src/
COPY CMakeLists.txt .

WORKDIR /senec2openwb/build
RUN cmake -DCMAKE_BUILD_TYPE=Release .. && cmake --build . --parallel 4

# senec2openwb run stage
FROM alpine:3.19.0
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