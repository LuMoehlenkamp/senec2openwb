# uses a multi-stage build strategy

# senec2openwb build stage

FROM --platform=$BUILDPLATFORM debian:bookworm AS build

RUN apt-get update && apt-get install -y python3 python3-pip python3-venv cmake build-essential gcc-aarch64-linux-gnu g++-aarch64-linux-gnu git ninja-build

RUN python3 -m venv /opt/venv
ENV PATH="/opt/venv/bin:$PATH"
RUN pip3 install conan
RUN conan --version
RUN conan profile detect --force

WORKDIR /senec2openwb
COPY src/ ./src/
COPY test/ ./test/
COPY CMakeLists.txt .
COPY conanfile.txt .

ARG TARGETPLATFORM
ARG BUILDPLATFORM

WORKDIR /senec2openwb/build
RUN conan install .. --build=missing
# --profile:host=default --profile:build=default
RUN cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=Release/generators/conan_toolchain.cmake ..
RUN cmake --build . --parallel 4

# ---------------------------------------------------------------------------------
FROM debian:bookworm-slim

RUN addgroup --system ludger && adduser --system --ingroup ludger ludger
USER ludger:ludger

COPY --chown=ludger:ludger --from=build ./senec2openwb/bin/senec2openwb /app/
RUN mkdir -p /app/dat
RUN chmod -R 755 /app/dat/
RUN mkdir -p /app/res
RUN chmod -R 755 /app/res/

WORKDIR /app
USER ludger:ludger
# RUN file /app/senec2openwb
ENTRYPOINT [ "./senec2openwb" ]