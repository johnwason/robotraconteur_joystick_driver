FROM ubuntu:jammy
WORKDIR /app
ENV DEBIAN_FRONTEND=noninteractive

RUN apt update && apt-get install wget sudo software-properties-common -y
RUN sudo add-apt-repository ppa:robotraconteur/ppa -y \
    && sudo apt-get update \
    && sudo apt-get install robotraconteur-dev librobotraconteur-companion-dev libsdl2-dev cmake build-essential \
    libdrekar-launch-process-cpp-dev -y

COPY . ./
COPY ./config/*.yml /config/

RUN cmake -B build -S . -DCMAKE_BUILD_TYPE=Release && cmake --build build
RUN cmake --install build

RUN cd / && rm -rf /app && apt autoremove -y && apt clean -y

ENV JOYSTICK_INFO_FILE=/config/joy0_default_config.yml
ENV JOYSTICK_ID=0

CMD exec /usr/local/bin/robotraconteur_joystick_driver --joystick-info-file=$JOYSTICK_INFO_FILE --joystick-id=$JOYSTICK_ID
