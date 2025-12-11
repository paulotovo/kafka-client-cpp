#include "producer.hpp" 
#include <iostream>
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <string>

enum class SecurityMode {
    PLAINTEXT,
    TLS,
    MTLS
};

int main(int argc, char* argv[]) {
    // 1. Verificação de Argumentos
    // Esperamos 4 argumentos: [Nome do Programa] [ip:porta] [topico] [mensagem]
    if (argc <= 4) {
        std::cerr << "Erro: Numero incorreto de argumentos." << std::endl;
        std::cerr << "Uso: " << argv[0] << " <ip:porta> <topico> <mensagem>" << std::endl;
        std::cerr << "Exemplo: " << argv[0] << " 192.168.56.111:9092 meu-topico 'mensagem' [--ssl-ca path-to-cert.pem --ssl-cert path-to-cert.pem --ssl-key path-to-cert.key ] " << std::endl;
        return 1;
    }

    // 2. Extração de Parâmetros
    const std::string brokers = argv[1];        // Endereço IP:Porta
    const std::string topic = argv[2];          // Tópico
    const std::string base_message = argv[3];   // Mensagem (Payload)
    
    // SSL Settings
    std::string ssl_ca="", ssl_cert="", ssl_key="";
    for (int i = 4; i < argc; i++) {
        std::string a = argv[i];
        if (a == "--ssl-ca")    ssl_ca  = argv[++i];
        else if (a == "--ssl-cert") ssl_cert = argv[++i];
        else if (a == "--ssl-key")  ssl_key  = argv[++i];
    }    

    // 3. Inicializa o Producer
    mykafka::Producer producer(brokers, ssl_ca, ssl_cert, ssl_key);

    // --- Geração do Timestamp ---
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    
    std::tm* local_tm = std::localtime(&now_time);
    
    std::ostringstream oss;
    oss << std::put_time(local_tm, "%Y-%m-%d %H:%M:%S");
    std::string timestamp_str = oss.str();
    
    // 4. Concatena a Mensagem Final
    std::string final_message = "[" + timestamp_str + "] " + base_message;

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
    
    // Opcional: Imprime o que será enviado
    std::cout << "Conectando em: " << brokers << std::endl;
    std::cout << "Conection Security mode: " << mode << std::endl; 
    
    // Imprime certificados
    if (secMode == SecurityMode::TLS || secMode == SecurityMode::MTLS) {
        std::cout << " → CA loaded: " << ssl_ca << "\n";
    }

    if (secMode == SecurityMode::MTLS) {
        std::cout << " → Client Cert: " << ssl_cert << "\n";
        std::cout << " → Client Key:  " << ssl_key << "\n";
    } 

    std::cout << "Enviando para o topico '" << topic << "': " << final_message << std::endl;
    
    // 5. Envia a Mensagem com Timestamp
    producer.send(topic, final_message);

    return 0;
}