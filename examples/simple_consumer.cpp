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

enum class SecurityMode {
    PLAINTEXT,
    TLS,
    MTLS
};

int main(int argc, char* argv[]) {
    // Verificação de Argumentos: Espera 4 argumentos
    if (argc <= 4) {
        std::cerr << "Erro: Número incorreto de argumentos." << std::endl;
        std::cerr << "Uso: " << argv[0] << " <ip:porta> <grupo_id> <topico1,topico2,...>" << std::endl;
        std::cerr << "Exemplo: " << argv[0] << " 192.168.56.111:9092 meu-app-grupo teste1,teste2 [--ssl-ca path-to-cert.pem --ssl-cert path-to-cert.pem --ssl-key path-to-cert.key ] " << std::endl;
        return 1;
    }

    // 1. Extração de Parâmetros
    const std::string brokers = argv[1];
    const std::string groupId = argv[2];
    const std::string topics_str = argv[3];

    // SSL Settings
    std::string ssl_ca="", ssl_cert="", ssl_key="";

    for (int i = 4; i < argc; i++) {
        std::string a = argv[i];
        if (a == "--ssl-ca")    ssl_ca  = argv[++i];
        else if (a == "--ssl-cert") ssl_cert = argv[++i];
        else if (a == "--ssl-key")  ssl_key  = argv[++i];
    }
    // -----------------------------------------------------

    // 2. Divisão dos Tópicos
    std::vector<std::string> topics = split_topics(topics_str, ',');

    if (topics.empty()) {
        std::cerr << "Erro: Nenhum tópico válido fornecido." << std::endl;
        return 1;
    }

    // Detecta modo TLS
    SecurityMode secMode;

    if (ssl_ca.empty() && ssl_cert.empty() && ssl_key.empty())
        secMode = SecurityMode::PLAINTEXT;
    else if (!ssl_ca.empty() && ssl_cert.empty() && ssl_key.empty())
        secMode = SecurityMode::TLS;
    else if (!ssl_ca.empty() && !ssl_cert.empty() && !ssl_key.empty())
        secMode = SecurityMode::MTLS;
    else
        secMode = SecurityMode::PLAINTEXT;

    std::string mode="";
    switch (secMode) {
        case SecurityMode::PLAINTEXT: mode = "PLAINTEXT"; break;
        case SecurityMode::TLS:       mode = "TLS (server-side only)"; break;
        case SecurityMode::MTLS:      mode = "mTLS (mutual authentication)"; break;
    }            

    // 3. Inicializa o Consumer
    // O construtor espera: brokers, groupId, {vector de tópicos}
    mykafka::Consumer consumer(brokers, groupId, topics, ssl_ca, ssl_cert, ssl_key);      
    
    std::cout << "Consumer iniciado no grupo '" << groupId << "'." << std::endl;
    std::cout << "Subscrito nos topicos: " << topics_str << std::endl;
    std::cout << "Conection Security mode: " << mode << std::endl; 

    // Imprime certificados
    if (secMode == SecurityMode::TLS || secMode == SecurityMode::MTLS) {
        std::cout << " → CA loaded: " << ssl_ca << "\n";
    }

    if (secMode == SecurityMode::MTLS) {
        std::cout << " → Client Cert: " << ssl_cert << "\n";
        std::cout << " → Client Key:  " << ssl_key << "\n";
    }    
    
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