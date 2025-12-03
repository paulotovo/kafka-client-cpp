# kafka_client_cpp
C++ Kafka client for real-time message sending (producer) and receiving (consumer).

-- meu-kafka-wrapper \
--- CMakeLists.txt \
--- include \
---- producer.hpp \
---- consumer.hpp \
--- src/ \
---- producer.cpp \
---- consumer.cpp \
--- examples/ \
----- simple_producer.cpp \
----- simple_consumer.cpp

# Examples

simple_producer: envia mensagens para meu-topico \
simple_consumer: escuta mensagens de meu-topico

# Pré-requisitos

Linux

g++, cmake, librdkafka-dev

sudo apt update
sudo apt install build-essential cmake librdkafka-dev

# Third Party

1. Pré Build (Dependencies)  

1.1 Build OpenSSL3 static
1.2 Build Zlib static
1.3 Build Lz4 static
```bash
git clone --branch v1.10.0 --depth 1 https://github.com/lz4/lz4.git
cd lz4
make lib
ls lib/libz4.a
```
1.4 Build zstd static

```bash
git clone --branch v1.5.7 --depth 1 https://github.com/facebook/zstd.git
cd zstd
mkdir _build && cd _build
cmake ../build/cmake -DZSTD_BUILD_SHARED=OFF -DZSTD_BUILD_STATIC=ON -DZSTD_BUILD_PROGRAMS=OFF -DZSTD_BUILD_TESTS=OFF
ls lib/libzstd.a
```
1.5 Build cURL
```bash
# Habilita o repositório CodeReady Builder
sudo dnf config-manager --set-enabled ol8_codeready_builder

# Tenta instalar o pacote libpsl-devel 
sudo dnf install libpsl-devel

sudo dnf install autoconf automake libtool m4
git clone --branch curl-8_12_1 --depth 1 https://github.com/curl/curl.git
cd curl
autoreconf -fi
unset CC CFLAGS LDFLAGS LIBS CPPFLAGS CXX CXXFLAGS CXXCPP CPP
export CFLAGS="-O3 -fPIC"

./configure --disable-debug --enable-static --disable-shared --with-pic --disable-ldap --without-libidn2 --without-zstd --with-zlib=$PWD/deps-release64 --with-ssl=$PWD/deps-release64 --prefix=$PWD/release64

# sem libpsl (FUNCIONA)
# compilar a curl com essa dependencia vai gerar problemas na rdkafka
./configure --disable-debug --enable-static --disable-shared --with-pic --without-libpsl --disable-ldap --without-libidn2 --without-zstd --with-zlib=$PWD/deps-release64 --with-ssl=$PWD/deps-release64 --prefix=$PWD/release64

make
make install

```

2. Download the librdkafka source code (GitHub oficial)

```bash
git clone https://github.com/confluentinc/librdkafka.git
cd librdkafka
```
2.1 Switch to version 2.12.1

```bash
git fetch --all --tags
git checkout v2.12.1
```

3. Build 
* The librdkafka CMake configuration finds the zstd library with system versions taking priority. Therefore, it is necessary to force it to search the path of the static zstd library.

```bash
mkdir _build && cd _build
cmake .. \
-DCMAKE_PREFIX_PATH=/mnt/data/totvs/github/kafka-client-cpp/third_party/zstd/_build \
-DCMAKE_INSTALL_PREFIX=/mnt/data/totvs/github/kafka-client-cpp/third_party/librdkafka/install \
-DRDKAFKA_BUILD_STATIC=ON \
-DWITH_SASL=OFF \
-DWITH_SSL=ON \
-DWITH_ZSTD=ON \
-DLZ4_LIBRARY=/mnt/data/totvs/github/kafka-client-cpp/third_party/lz4/lib/liblz4.a \
-DLZ4_INCLUDE_DIR=/mnt/data/totvs/github/kafka-client-cpp/third_party/lz4/lib \
-DZSTD_LIBRARY=/mnt/data/totvs/github/kafka-client-cpp/third_party/zstd/_build/lib/libzstd.a \
-DZSTD_INCLUDE_DIR=/mnt/data/totvs/github/kafka-client-cpp/third_party/zstd/_build/lib \
-DCURL_LIBRARY=/mnt/data/totvs/github/kafka-client-cpp/third_party/curl/release64/lib/libcurl.a \
-DCURL_INCLUDE_DIR=/mnt/data/totvs/github/kafka-client-cpp/third_party/curl/release64/include \
-DOPENSSL_ROOT_DIR=/mnt/data/totvs/github/kafka-client-cpp/third_party/openssl/3.0.16/linux64/gcc-11.2.1 \
-DOPENSSL_CRYPTO_LIBRARY=/mnt/data/totvs/github/kafka-client-cpp/third_party/openssl/3.0.16/linux64/gcc-11.2.1/release/libcrypto.a \
-DOPENSSL_SSL_LIBRARY=/mnt/data/totvs/github/kafka-client-cpp/third_party/openssl/3.0.16/linux64/gcc-11.2.1/release/libssl.a \
-DZLIB_LIBRARY=/mnt/data/totvs/github/kafka-client-cpp/third_party/zlib/1.2.13/linux64/gcc-11.2.1/release/libzlib.a \
-DZLIB_INCLUDE_DIR=/mnt/data/totvs/github/kafka-client-cpp/third_party/zlib/1.2.13/linux64/gcc-11.2.1/include \
-DCMAKE_POSITION_INDEPENDENT_CODE=ON

make
make install
ls src/librdkafka.a
```

# Compilando o Projeto

Crie e entre na pasta de build:

mkdir build && cd build

Configure com CMake:

cmake .. \
-DRDKAFKA_ROOT=/mnt/data/totvs/github/kafka-client-cpp/third_party/librdkafka/install \
-DRDKAFKA_LIB=/mnt/data/totvs/github/kafka-client-cpp/third_party/librdkafka/install/lib64/librdkafka.a \
-DRDKAFKA_INCLUDE=/mnt/data/totvs/github/kafka-client-cpp/third_party/librdkafka/install/include \
-DLZ4_LIB=/mnt/data/totvs/github/kafka-client-cpp/third_party/lz4/lib/liblz4.a \
-DLZ4_INCLUDE=/mnt/data/totvs/github/kafka-client-cpp/third_party/lz4/lib \
-DZSTD_LIB=/mnt/data/totvs/github/kafka-client-cpp/third_party/zstd/_build/lib/libzstd.a \
-DZSTD_INCLUDE=/mnt/data/totvs/github/kafka-client-cpp/third_party/zstd/include \
-DZLIB_LIB=/mnt/data/totvs/github/kafka-client-cpp/third_party/zlib/1.2.13/linux64/gcc-11.2.1/release/libzlib.a \
-DZLIB_INCLUDE=/mnt/data/totvs/github/kafka-client-cpp/third_party/zlib/1.2.13/linux64/gcc-11.2.1/include \
-DOPENSSL_SSL_LIB=/mnt/data/totvs/github/kafka-client-cpp/third_party/openssl/3.0.16/linux64/gcc-11.2.1/release/libssl.a \
-DOPENSSL_CRYPTO_LIB=/mnt/data/totvs/github/kafka-client-cpp/third_party/openssl/3.0.16/linux64/gcc-11.2.1/release/libcrypto.a \
-DOPENSSL_INCLUDE=/mnt/data/totvs/github/kafka-client-cpp/third_party/openssl/3.0.16/linux64/gcc-11.2.1/include \
-DCURL_LIB=/mnt/data/totvs/github/kafka-client-cpp/third_party/curl/release64/lib/libcurl.a \
-DCURL_INCLUDE=/mnt/data/totvs/github/kafka-client-cpp/third_party/curl/release64/include 

Execute os exemplos:

./examples/simple_producer \
./examples/simple_consumer

