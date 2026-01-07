#pragma once

#include <string>
#include <memory>
#include <functional>
#include <unordered_map>

namespace mykafka {

#ifdef ASYNC_MODE
    struct DeliveryReport {
       bool success;
       std::string error;
       int32_t partition; // Partição 0 → offsets 0,1,2,3...
       int64_t offset; // é o número da posição da mensagem dentro de uma partição do tópico.
    };
#endif
    

class Producer {
public:

#ifdef ASYNC_MODE
    using DeliveryCallback = std::function<void(const DeliveryReport&)>;   
#else
    using DeliveryCallback = std::function<void(bool success, const std::string& error)>;
#endif
    Producer(const std::string& brokers, const std::string& ssl_ca   = "", const std::string& ssl_cert = "", const std::string& ssl_key  = "");
    ~Producer();

    // permite configurar SSL, timeouts, etc
    //void setConfig(const std::string& key, const std::string& value);    

    void send(const std::string& topic, const std::string& message, DeliveryCallback callback = nullptr);

#ifdef ASYNC_MODE
    void send_async(const std::string& topic, const std::string& message, DeliveryCallback callback);
#endif

    void flush(int timeout_ms = 1000);

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace mykafka
