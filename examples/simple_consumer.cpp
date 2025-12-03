#include "consumer.hpp"
#include <iostream>

int main() {
    mykafka::Consumer consumer("192.168.56.111:9092", "meu-grupo", {"test"});
    std::cout << "Consumer iniciado. Pressione Ctrl+C para sair." << std::endl;

    // Adicione um loop forçado para rodar o consumer
    while (true) { 
        consumer.poll([](const std::string& msg) {
            std::cout << "Recebido: " << msg << std::endl;
        });
    }

    return 0; // Este código nunca será alcançado, exceto por um erro fatal.
}