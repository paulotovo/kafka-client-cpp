#pragma once

#include <string>
#include <vector>
#include <functional>
#include <memory>

namespace mykafka {

class Consumer {
public:
    //using MessageCallback = std::function<void(const std::string& message)>;
    //Adicionar 'const std::string& topic' ao callback
    using MessageCallback = std::function<void(const std::string& topic, const std::string& message)>;    

    Consumer(const std::string& brokers,
            const std::string& groupId,
            const std::vector<std::string>& topics,
            const std::string& ssl_ca   = "",
            const std::string& ssl_cert = "",
            const std::string& ssl_key  = "");
    ~Consumer();

    void poll(MessageCallback callback, int timeout_ms = 1000);

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace mykafka
