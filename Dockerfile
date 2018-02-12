FROM debian:stable-slim

RUN apt-get update &&apt-get install -y --fix-missing \
  build-essential\
  git\
  cmake \
  libboost1.62-all-dev \
  libsigc++-2.0-dev \
  libssl-dev \
  libxml++2.6-dev \
  libxml++2.6-doc \
  libcpprest2.9 \
  uuid-dev\
  libcpprest-dev \
  libunittest++-dev
# Build azure-storage-cpp, then test to make sure it was built correctly
RUN git clone https://github.com/bjornefitte/azure-storage-cpp.git && \
  cd azure-storage-cpp/Microsoft.WindowsAzure.Storage && \
  mkdir build.release && \
  cd build.release && \
  CASABLANCA_DIR=$(find /usr/lib -name libcpprest.so) cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-Wno-deprecated-declarations" -DBUILD_TESTS=ON && \
  make
