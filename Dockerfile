FROM ubuntu:22.04

ARG BUILD_TYPE=Debug

RUN mkdir -p /usr/src/ecsify
WORKDIR /usr/src/ecsify

RUN apt update
RUN apt install -y \
                python3 python3-pip \
                cmake clang-format clang-tidy \
                lcov

COPY requirements.txt ./
RUN pip3 install -r requirements.txt
COPY conanfile.txt ./
RUN conan profile detect && conan install . --build=missing -s build_type=${BUILD_TYPE}

COPY . ./
