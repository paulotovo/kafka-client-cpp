#pragma once

#include <string>
#include <memory>
#include <functional>

namespace mykafka {

class Producer {
public:
    using DeliveryCallback = std::function<void(bool success, const std::string& error)>;

    Producer(const std::string& brokers);
    ~Producer();

    void send(const std::string& topic, const std::string& message, DeliveryCallback callback = nullptr);

    void flush(int timeout_ms = 1000);

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace mykafka
