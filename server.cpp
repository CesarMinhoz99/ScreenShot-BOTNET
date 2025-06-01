#include <iostream>     // Entrada e saída padrão (cout, cerr)
#include <fstream>      // Manipulação de arquivos (escrita binária)
#include <asio.hpp>     // Biblioteca ASIO para comunicação TCP/IP

using asio::ip::tcp;    // Facilita o uso do tcp::socket e tcp::acceptor

// Para compilar (exemplo Windows):
// g++ -std=c++17 -IC:/Users/Cesar/Desktop/avbot/include/ server.cpp -o server -lws2_32 -lgdi32

int main() {
    try {
        asio::io_context io; // Contexto de entrada/saída do ASIO

        // Cria um acceptor TCP que escuta na porta 12345 para IPv4
        tcp::acceptor acceptor(io, tcp::endpoint(tcp::v4(), 12345));
        std::cout << "servidor> Aguardando bot...\n";

        tcp::socket socket(io);   // Cria um socket TCP para a conexão
        acceptor.accept(socket);  // Aguarda conexão de cliente e aceita
        std::cout << "servidor> bot conectado.\n";

        int count = 0;  // Contador para nomear arquivos recebidos

        // Loop infinito para receber imagens continuamente
        while (true) {
            size_t imageSize;

            // Lê do socket o tamanho da imagem (quantidade de bytes)
            asio::read(socket, asio::buffer(&imageSize, sizeof(imageSize)));

            // Cria um buffer com o tamanho exato para armazenar a imagem
            std::vector<char> buffer(imageSize);

            // Lê os dados da imagem enviados pelo cliente
            asio::read(socket, asio::buffer(buffer));

            // Gera nome do arquivo com contador, ex: received_0.bmp
            std::string filename = "received_" + std::to_string(count++) + ".bmp";

            // Abre arquivo para escrita binária
            std::ofstream out(filename, std::ios::binary);

            // Escreve os dados da imagem no arquivo
            out.write(buffer.data(), buffer.size());

            // Fecha o arquivo
            out.close();

            // Informa no console que a imagem foi salva
            std::cout << "servidor> Imagem salva como " << filename << "\n";
        }

    } catch (std::exception& e) {
        // Se ocorrer erro, mostra mensagem no console
        std::cerr << "servidor> Erro: " << e.what() << std::endl;
    }

    return 0;  // Fim do programa
}
