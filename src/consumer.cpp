#include "consumer.hpp"
#include <librdkafka/rdkafka.h>
#include <stdexcept>
#include <iostream>

namespace mykafka {

class Consumer::Impl {
public:
    rd_kafka_t* rk;
    rd_kafka_conf_t* conf;
    rd_kafka_topic_partition_list_t* topic_list;

    Impl(const std::string& brokers,
         const std::string& groupId,
         const std::vector<std::string>& topics)
    {
        char errstr[512];

        conf = rd_kafka_conf_new();

        // brokers
        if (rd_kafka_conf_set(conf, "bootstrap.servers", brokers.c_str(),
                              errstr, sizeof(errstr)) != RD_KAFKA_CONF_OK) {
            throw std::runtime_error(errstr);
        }

        // group.id
        if (rd_kafka_conf_set(conf, "group.id", groupId.c_str(),
                              errstr, sizeof(errstr)) != RD_KAFKA_CONF_OK) {
            throw std::runtime_error(errstr);
        }

        // auto offset reset
        rd_kafka_conf_set(conf, "auto.offset.reset", "earliest",
                          nullptr, 0);

        // cria consumer
        rk = rd_kafka_new(RD_KAFKA_CONSUMER, conf, errstr, sizeof(errstr));
        if (!rk)
            throw std::runtime_error(std::string("Erro criando consumer: ") + errstr);

        // registra consumer como consumidor
        rd_kafka_poll_set_consumer(rk);

        // lista de tópicos
        topic_list = rd_kafka_topic_partition_list_new(topics.size());
        for (const auto& t : topics) {
            rd_kafka_topic_partition_list_add(topic_list, t.c_str(), RD_KAFKA_PARTITION_UA);
        }

        // subscribe
        rd_kafka_resp_err_t err = rd_kafka_subscribe(rk, topic_list);
        if (err != RD_KAFKA_RESP_ERR_NO_ERROR) {
            throw std::runtime_error(rd_kafka_err2str(err));
        }
    }

    ~Impl() {
        rd_kafka_unsubscribe(rk);
        rd_kafka_consumer_close(rk);
        rd_kafka_topic_partition_list_destroy(topic_list);
        rd_kafka_destroy(rk);
    }

    void poll(Consumer::MessageCallback callback, int timeout_ms) {
        rd_kafka_message_t* msg = rd_kafka_consumer_poll(rk, timeout_ms);
        if (!msg) return;

        if (msg->err == RD_KAFKA_RESP_ERR_NO_ERROR) {
            // 1. EXTRAIR O NOME DO TÓPICO
            std::string topic_name(rd_kafka_topic_name(msg->rkt)); // O rkt é o rd_kafka_topic_t*

            // 2. PASSAR O NOME DO TÓPICO E A MENSAGEM PARA O CALLBACK
            if (callback) {
                callback(topic_name, std::string((char*)msg->payload, msg->len));
            }
        } else if (msg->err != RD_KAFKA_RESP_ERR__PARTITION_EOF &&
                   msg->err != RD_KAFKA_RESP_ERR__TIMED_OUT) {
            std::cerr << "Erro ao consumir: "
                      << rd_kafka_message_errstr(msg) << std::endl;
        }

        rd_kafka_message_destroy(msg);
    }
};

Consumer::Consumer(const std::string& brokers,
                   const std::string& groupId,
                   const std::vector<std::string>& topics)
    : impl_(std::make_unique<Impl>(brokers, groupId, topics))
{
}

Consumer::~Consumer() = default;

void Consumer::poll(MessageCallback callback, int timeout_ms) {
    impl_->poll(callback, timeout_ms);
}

} // namespace mykafka
