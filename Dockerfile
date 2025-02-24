FROM ubuntu:22.04

RUN mkdir -p /usr/src/ecsify
WORKDIR /usr/src/ecsify

RUN apt update
RUN apt install -y \
                python3 python3-pip \
                cmake clang-format clang-tidy

COPY requirements.txt conanfile.txt ./
RUN pip3 install -r requirements.txt
RUN conan profile detect && conan install . --build=missing

COPY . ./
