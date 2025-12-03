#include "producer.hpp"

int main() {
    mykafka::Producer producer("192.168.56.111:9092");
    producer.send("test", "Mensagem teste!");
}