#include "producer.hpp"
#include <librdkafka/rdkafka.h>
#include <stdexcept>
#ifdef ASYNC_MODE
  #include <thread>
  #include <atomic>
#endif
#include <iostream>

namespace mykafka {

class Producer::Impl {
public:
    rd_kafka_t* rk{};
    rd_kafka_conf_t* conf{};
#ifdef ASYNC_MODE    
    rd_kafka_queue_t* queue{};
    std::thread event_thread;
    std::atomic<bool> running{true}; 
#endif   

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
        
#ifdef ASYNC_MODE
        rd_kafka_conf_set_dr_msg_cb(conf, dr_msg_cb);
#endif
        
        // -----------------------------------

        rk = rd_kafka_new(RD_KAFKA_PRODUCER, conf, errstr, sizeof(errstr));
        if (!rk) 
          throw std::runtime_error(errstr);

#ifdef ASYNC_MODE
        // FORÇA a librdkafka a enviar relatórios de entrega para a fila principal 
        // que o rd_kafka_poll(rk) consome.
        rd_kafka_set_log_level(rk, 7); // Debug (opcional para ver o que ocorre)

        // A librdkafka já tem uma fila principal. Usamos o poll(rk) nela.
        event_thread = std::thread([this]() {
            while (running || rd_kafka_outq_len(rk) > 0) 
            {   
                // Garante processar pendentes
                rd_kafka_poll(rk, 100); 
                std::cout << "Passou aqui na thread" << std::endl;
            }
        });
#endif        
    }

    ~Impl() 
    {
#ifdef ASYNC_MODE
        running = false;   // Avisa a thread para parar, mas ela verificará a fila no loop acima
        // Aguarda a thread de eventos processar os últimos callbacks
        if (event_thread.joinable())
            event_thread.join();
#endif
        // O flush força o envio e internamente chama o poll para os callbacks pendentes
        // Se o outq_len já for 0 (porque o usuário chamou flush antes), 
        // este rd_kafka_flush retornará instantaneamente.
        if (rd_kafka_outq_len(rk) > 0) {
            rd_kafka_flush(rk, 3000); 
        }

        rd_kafka_destroy(rk);
    }

    void send(const std::string& topic, const std::string& message, Producer::DeliveryCallback callback)
    {
#ifdef ASYNC_MODE
        send_async(topic, message, callback);
#else
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
#endif
    }

#ifdef ASYNC_MODE
void send_async(const std::string& topic, const std::string& message, DeliveryCallback callback) 
{
    auto* cb_ptr = new mykafka::Producer::DeliveryCallback(std::move(callback));

    // Capturamos o retorno para saber se a mensagem foi aceita para envio
    rd_kafka_resp_err_t err = rd_kafka_producev(
            rk,
            RD_KAFKA_V_TOPIC(topic.c_str()),
            RD_KAFKA_V_MSGFLAGS(RD_KAFKA_MSG_F_COPY),
            RD_KAFKA_V_VALUE((void*)message.data(), message.size()),
            RD_KAFKA_V_OPAQUE(cb_ptr), // Macro correta para a API producev
            RD_KAFKA_V_END
        );

    // Se err != 0, a librdkafka NÃO chamará o dr_msg_cb. 
    // Precisamos tratar o erro e limpar a memória manualmente aqui.
    if (err != RD_KAFKA_RESP_ERR_NO_ERROR) {
        DeliveryReport report;
        report.success = false;
        report.error = rd_kafka_err2str(err);
        report.partition = -1;
        report.offset = -1;
        
        if (cb_ptr) {
            (*cb_ptr)(report);
            delete cb_ptr;
        }
    }
}
#endif

    void flush(int timeout_ms) {
        rd_kafka_flush(rk, timeout_ms);
    }

#ifdef ASYNC_MODE
    static void dr_msg_cb(rd_kafka_t*,
                        const rd_kafka_message_t* msg,
                        void* opaque) {
        std::cout << "[DEBUG] dr_msg_cb disparado!" << std::endl;
        std::cout << "[DEBUG] Ponteiro opaque recebido: " << opaque << std::endl;

        // O seu cb_ptr (DeliveryCallback) está aqui:
        void* message_opaque = msg->_private; 

        if (!message_opaque) 
        {
            // Isso pode acontecer se você enviar uma mensagem sem callback
            return;
        }        
        
        auto* cb = static_cast<mykafka::Producer::DeliveryCallback*>(message_opaque);
        //auto* cb = static_cast<mykafka::Producer::DeliveryCallback*>(opaque);
        if (!cb) 
        {
            std::cout << "[ERRO] Opaque nulo no callback!" << std::endl;
            return;
        }

        DeliveryReport report;
        report.success   = (msg->err == RD_KAFKA_RESP_ERR_NO_ERROR);
        report.error     = rd_kafka_err2str(msg->err);
        report.partition = msg->partition;
        report.offset    = msg->offset;

        (*cb)(report);
        delete cb;
    }
#endif    
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
