FROM ubuntu:22.04

RUN apt update && apt install -y python3 python3-pip cmake wget

RUN echo "deb http://apt.llvm.org/jammy/ llvm-toolchain-jammy-18 main" > /etc/apt/sources.list.d/llvm.list
RUN echo "deb-src http://apt.llvm.org/jammy/ llvm-toolchain-jammy-18 main" >> /etc/apt/sources.list.d/llvm.list
RUN wget -qO- https://apt.llvm.org/llvm-snapshot.gpg.key > /etc/apt/trusted.gpg.d/apt.llvm.org.asc
RUN apt update && apt install -y clang-format-18 clang-tidy-18

RUN mkdir -p /usr/src/ecsify
WORKDIR /usr/src/ecsify

COPY requirements.txt ./
RUN pip3 install -r requirements.txt
COPY conanfile.txt ./
RUN conan profile detect
RUN conan install . --build=missing -s build_type=Debug -s compiler.cppstd=20
RUN conan install . --build=missing -s build_type=Release -s compiler.cppstd=20

COPY . ./
