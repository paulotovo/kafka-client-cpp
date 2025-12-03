#include "producer.hpp"
#include <iostream>
#include <chrono>     // Para manipular tempo
#include <ctime>      // Para time_t e localtime
#include <sstream>    // Para formatar a string
#include <iomanip>    // Para put_time
#include <string>     // Para manipulação de strings

int main() {
    mykafka::Producer producer("192.168.56.111:9092");

    // --- Geração do Timestamp ---
    // Obtém o tempo atual do sistema
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    
    // Converte para uma estrutura de tempo local
    std::tm* local_tm = std::localtime(&now_time);
    
    // Formata o timestamp como uma string (AAAA-MM-DD HH:MM:SS)
    std::ostringstream oss;
    oss << std::put_time(local_tm, "%Y-%m-%d %H:%M:%S");
    std::string timestamp_str = oss.str();
    
    // 2. Concatena a Mensagem
    std::string base_message = "Mensagem teste!";
    std::string final_message = "[" + timestamp_str + "] " + base_message;
    
    // Opcional: Imprime para debug
    std::cout << "Enviando: " << final_message << std::endl;
    
    // 3. Envia a Mensagem com Timestamp
    producer.send("test", final_message);
}