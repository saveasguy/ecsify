FROM ubuntu:22.04

RUN apt update && apt install -y python3 python3-pip \
    wget lsb-release gnupg software-properties-common

RUN wget https://apt.llvm.org/llvm.sh
RUN chmod +x llvm.sh
RUN ./llvm.sh 18 all

RUN mkdir -p /usr/src/ecsify
WORKDIR /usr/src/ecsify

COPY requirements.txt ./
RUN pip3 install -r requirements.txt
COPY conanfile.txt ./
COPY profiles ./profiles
RUN conan profile detect
RUN conan install . --build=missing -pr ./profiles/clang18
RUN conan install . --build=missing -pr ./profiles/clang18 -s build_type=Debug

COPY . ./
