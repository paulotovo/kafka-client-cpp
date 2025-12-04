#include "producer.hpp" 
#include <iostream>
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <string>

int main(int argc, char* argv[]) {
    // 1. Verificação de Argumentos
    // Esperamos 4 argumentos: [Nome do Programa] [ip:porta] [topico] [mensagem]
    if (argc != 4) {
        std::cerr << "Erro: Numero incorreto de argumentos." << std::endl;
        std::cerr << "Uso: " << argv[0] << " <ip:porta> <topico> <mensagem>" << std::endl;
        std::cerr << "Exemplo: " << argv[0] << " 192.168.56.111:9092 meu-topico 'mensagem'" << std::endl;
        return 1;
    }

    // 2. Extração de Parâmetros
    const std::string brokers = argv[1];        // Endereço IP:Porta
    const std::string topic = argv[2];          // Tópico
    const std::string base_message = argv[3];   // Mensagem (Payload)

    // 3. Inicializa o Producer
    mykafka::Producer producer(brokers);

    // --- Geração do Timestamp ---
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    
    std::tm* local_tm = std::localtime(&now_time);
    
    std::ostringstream oss;
    oss << std::put_time(local_tm, "%Y-%m-%d %H:%M:%S");
    std::string timestamp_str = oss.str();
    
    // 4. Concatena a Mensagem Final
    std::string final_message = "[" + timestamp_str + "] " + base_message;
    
    // Opcional: Imprime o que será enviado
    std::cout << "Conectando em: " << brokers << std::endl;
    std::cout << "Enviando para o topico '" << topic << "': " << final_message << std::endl;
    
    // 5. Envia a Mensagem com Timestamp
    producer.send(topic, final_message);

    return 0;
}