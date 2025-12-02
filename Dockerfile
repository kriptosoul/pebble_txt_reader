FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential wget curl ca-certificates python2 python2-dev python-is-python2 \
    libfreetype6-dev libpng-dev libusb-1.0-0-dev libxrender1 libxext6 unzip bzip2 git \
    && rm -rf /var/lib/apt/lists/*

# ARM toolchain
RUN wget -q https://developer.arm.com/-/media/Files/downloads/gnu-rm/4_9-2015q3/gnueabi/gcc-arm-none-eabi-4_9-2015q3-20150921-linux.tar.bz2 \
    && tar -xf gcc-arm-none-eabi-4_9-2015q3-20150921-linux.tar.bz2 -C /opt \
    && rm gcc-arm-none-eabi-4_9-2015q3-20150921-linux.tar.bz2
ENV PATH="/opt/gcc-arm-none-eabi-4_9-2015q3/bin:$PATH"

# Pebble SDK classic (contains pebble tool)
RUN wget -q https://github.com/pebble-dev/pebble-tool/releases/download/v4.7.0/pebble-sdk-4.7-linux64.tar.bz2 \
    && tar -xf pebble-sdk-4.7-linux64.tar.bz2 -C /opt \
    && rm pebble-sdk-4.7-linux64.tar.bz2
ENV PEBBLE_SDK_PATH=/opt/pebble-sdk-4.7-linux64
ENV PATH="$PEBBLE_SDK_PATH/bin:$PATH"

WORKDIR /work
CMD ["/bin/bash"]
