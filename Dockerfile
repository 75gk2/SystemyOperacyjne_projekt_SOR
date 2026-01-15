FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    gdb \
    ninja-build \
    git \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

RUN useradd -m dev
USER dev

WORKDIR /work


#docker build -t clion-dev .
#--rm --cpus=4 --memory=8g --memory-swap=8g
