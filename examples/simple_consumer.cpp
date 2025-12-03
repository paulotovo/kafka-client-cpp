#include "consumer.hpp"
#include <iostream>
#include <string>
#include <sstream>
#include <vector>

// Função auxiliar para dividir a string de tópicos
std::vector<std::string> split_topics(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) {
        if (!token.empty()) {
            tokens.push_back(token);
        }
    }
    return tokens;
}

int main(int argc, char* argv[]) {
    // Verificação de Argumentos: Espera 4 argumentos
    if (argc != 4) {
        std::cerr << "Erro: Número incorreto de argumentos." << std::endl;
        std::cerr << "Uso: " << argv[0] << " <ip:porta> <grupo_id> <topico1,topico2,...>" << std::endl;
        std::cerr << "Exemplo: " << argv[0] << " 192.168.56.111:9092 meu-app-grupo teste1,teste2" << std::endl;
        return 1;
    }

    // 1. Extração de Parâmetros
    const std::string brokers = argv[1];
    const std::string groupId = argv[2];
    const std::string topics_str = argv[3];

    // 2. Divisão dos Tópicos
    std::vector<std::string> topics = split_topics(topics_str, ',');

    if (topics.empty()) {
        std::cerr << "Erro: Nenhum tópico válido fornecido." << std::endl;
        return 1;
    }

    // 3. Inicializa o Consumer
    // O construtor espera: brokers, groupId, {vector de tópicos}
    mykafka::Consumer consumer(brokers, groupId, topics);
    
    std::cout << "Consumer iniciado no grupo '" << groupId << "'." << std::endl;
    std::cout << "Subscrito nos topicos: " << topics_str << std::endl;
    std::cout << "Pressione Ctrl+C para sair." << std::endl;

    // 4. Loop de Consumo (Agora com identificação de tópico)
    while (true) { 
        // A função de callback agora espera (topic, msg)
        consumer.poll([](const std::string& topic_name, const std::string& msg) {
            // Aqui a mensagem é distinguida
            std::cout << "[" << topic_name << "] Recebido: " << msg << std::endl;
        });
    }

    return 0; 
}