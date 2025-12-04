cmake_minimum_required(VERSION 3.10)
project(meu_kafka_wrapper)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Inclui headers do wrapper
include_directories(include)

# -------------------------------------------------------------------
# 1) Caminhos das libs estáticas que você gerou
# ajuste conforme sua árvore de diretórios
# -------------------------------------------------------------------
set(RDKAFKA_ROOT "/mnt/data/totvs/github/kafka-client-cpp/third_party/librdkafka/install")
set(RDKAFKA_LIB   "${RDKAFKA_ROOT}/lib64/librdkafka.a")
set(RDKAFKA_INCLUDE "${RDKAFKA_ROOT}/include")   # onde ficam os headers rdkafka

set(LZ4_LIB   "/mnt/data/totvs/github/kafka-client-cpp/third_party/lz4/lib/liblz4.a")
set(LZ4_INCLUDE "/mnt/data/totvs/github/kafka-client-cpp/third_party/lz4/lib")

set(ZSTD_LIB  "/mnt/data/totvs/github/kafka-client-cpp/third_party/zstd/_build/lib/libzstd.a")
set(ZSTD_INCLUDE "/mnt/data/totvs/github/kafka-client-cpp/third_party/zstd/include")

set(ZLIB_LIB  "/mnt/data/totvs/github/kafka-client-cpp/third_party/zlib/1.2.13/linux64/gcc-11.2.1/release/libzlib.a")
set(ZLIB_INCLUDE "/mnt/data/totvs/github/kafka-client-cpp/third_party/zlib/1.2.13/linux64/gcc-11.2.1/include")

set(OPENSSL_SSL_LIB   "/mnt/data/totvs/github/kafka-client-cpp/third_party/openssl/3.0.16/linux64/gcc-11.2.1/release/libssl.a")
set(OPENSSL_CRYPTO_LIB "/mnt/data/totvs/github/kafka-client-cpp/third_party/openssl/3.0.16/linux64/gcc-11.2.1/release/libcrypto.a")
set(OPENSSL_INCLUDE "/mnt/data/totvs/github/kafka-client-cpp/third_party/openssl/3.0.16/linux64/gcc-11.2.1/include")

# libcurl (estática) que você compilou
set(CURL_LIB "/mnt/data/totvs/github/kafka-client-cpp/third_party/curl/release64/lib/libcurl.a")
set(CURL_INCLUDE "/mnt/data/totvs/github/kafka-client-cpp/third_party/curl/release64/include")

# Se o libcurl foi compilado com libssh2, descomente e ajuste:
# set(LIBSSH2_LIB "/caminho/para/libssh2.a")
# set(LIBSSH2_INCLUDE "/caminho/para/libssh2/include")

# pthread, dl, m, rt são frequentemente necessários para link estático
set(EXTRA_LIBS pthread dl m rt)

# inclui headers do librdkafka
include_directories(${RDKAFKA_INCLUDE} ${OPENSSL_INCLUDE} ${ZSTD_INCLUDE} ${LZ4_INCLUDE} ${ZLIB_INCLUDE} ${CURL_INCLUDE})

# -------------------------------------------------------------------
# 2) Cria a lib do seu wrapper
# -------------------------------------------------------------------
add_library(mykafka
    src/producer.cpp
    src/consumer.cpp
)

# -------------------------------------------------------------------
# 3) Ordem de link (IMPORTANTE para archives estáticos):
#    - Primeiro a lib que seu wrapper usa diretamente (librdkafka.a)
#    - Em seguida as libs das quais o librdkafka depende
#    - Por fim libs do sistema (pthread, dl, m, rt)
# -------------------------------------------------------------------
target_link_libraries(mykafka PRIVATE
    ${RDKAFKA_LIB}
    ${CURL_LIB}          # se você quer suporte HTTP/OAuth, senão remova
    ${OPENSSL_SSL_LIB}
    ${OPENSSL_CRYPTO_LIB}
    ${ZSTD_LIB}
    ${LZ4_LIB}
    ${ZLIB_LIB}
    # ${LIBSSH2_LIB}    # habilite somente se seu libcurl precisar
    ${EXTRA_LIBS}
)

# -------------------------------------------------------------------
# 4) Executáveis de exemplo
# -------------------------------------------------------------------
add_executable(simple_producer examples/simple_producer.cpp)
target_link_libraries(simple_producer PRIVATE mykafka)

add_executable(simple_consumer examples/simple_consumer.cpp)
target_link_libraries(simple_consumer PRIVATE mykafka)
