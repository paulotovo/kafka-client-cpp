#pragma once

#include <string>
#include <memory>
#include <functional>
#include <unordered_map>

namespace mykafka {

class Producer {
public:
    using DeliveryCallback = std::function<void(bool success, const std::string& error)>;

    Producer(const std::string& brokers, const std::string& ssl_ca   = "", const std::string& ssl_cert = "", const std::string& ssl_key  = "");
    ~Producer();

    // permite configurar SSL, timeouts, etc
    void setConfig(const std::string& key, const std::string& value);    

    void send(const std::string& topic, const std::string& message, DeliveryCallback callback = nullptr);

    void flush(int timeout_ms = 1000);

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace mykafka
