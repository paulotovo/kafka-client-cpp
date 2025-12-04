cmake_minimum_required(VERSION 3.10)
project(meu_kafka_wrapper)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Inclui headers do wrapper
include_directories(include)

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
