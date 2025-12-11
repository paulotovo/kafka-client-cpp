#include "producer.hpp"
#include <librdkafka/rdkafka.h>
#include <stdexcept>
#include <iostream>

namespace mykafka {

class Producer::Impl {
public:
    rd_kafka_t* rk;
    rd_kafka_conf_t* conf;

    Impl(const std::string& brokers, const std::string& ssl_ca, const std::string& ssl_cert, const std::string& ssl_key) 
    {
        char errstr[512];

        conf = rd_kafka_conf_new();

        // bootstrap
        if (rd_kafka_conf_set(conf, "bootstrap.servers",
                              brokers.c_str(),
                              errstr, sizeof(errstr)) != RD_KAFKA_CONF_OK)
        {
            throw std::runtime_error(errstr);
        }

        // --- SSL (somente se fornecido) ---
        if (!ssl_ca.empty()) {
            rd_kafka_conf_set(conf, "security.protocol", "SSL", nullptr, 0);
            // Desabilitar a validação de hostname se você usou IP no certificado.
            // Certificado do broker usa apenas o IP no CN/SAN e nao um hostname valido. 
            rd_kafka_conf_set(conf, "ssl.endpoint.identification.algorithm", "none", nullptr, 0);            
            rd_kafka_conf_set(conf, "ssl.ca.location", ssl_ca.c_str(), nullptr, 0);
        }

        if (!ssl_cert.empty()) {
            rd_kafka_conf_set(conf, "ssl.certificate.location",
                            ssl_cert.c_str(), nullptr, 0);
        }

        if (!ssl_key.empty()) {
            rd_kafka_conf_set(conf, "ssl.key.location",
                            ssl_key.c_str(), nullptr, 0);
        }
        // -----------------------------------        

        rk = rd_kafka_new(RD_KAFKA_PRODUCER, conf, errstr, sizeof(errstr));
        if (!rk) throw std::runtime_error(errstr);
    }

    ~Impl() {
        rd_kafka_flush(rk, 1000);
        rd_kafka_destroy(rk);
    }

    void send(const std::string& topic,
              const std::string& message,
              Producer::DeliveryCallback callback)
    {
        rd_kafka_topic_t* rkt = rd_kafka_topic_new(rk, topic.c_str(), nullptr);
        if (!rkt)
            throw std::runtime_error("Failed to create topic");

        int err = rd_kafka_produce(
            rkt,
            RD_KAFKA_PARTITION_UA,
            RD_KAFKA_MSG_F_COPY,
            (void*)message.c_str(),
            message.size(),
            nullptr,
            0,
            nullptr);

        rd_kafka_poll(rk, 0);
        rd_kafka_topic_destroy(rkt);

        if (callback) {
            if (err != 0)
                callback(false, rd_kafka_err2str(rd_kafka_last_error()));
            else
                callback(true, "");
        }
    }

    void flush(int timeout_ms) {
        rd_kafka_flush(rk, timeout_ms);
    }
};

// ---------- Producer ----------

Producer::Producer(const std::string& brokers,
                   const std::string& ssl_ca,
                   const std::string& ssl_cert,
                   const std::string& ssl_key)
    : impl_(std::make_unique<Impl>(brokers, ssl_ca, ssl_cert, ssl_key)) 
{
    
}

Producer::~Producer() = default;

void Producer::send(const std::string& topic,
                    const std::string& message,
                    DeliveryCallback callback)
{
    impl_->send(topic, message, callback);
}

void Producer::flush(int timeout_ms)
{
    impl_->flush(timeout_ms);
}

} // namespace mykafka
