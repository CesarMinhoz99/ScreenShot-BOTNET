#include <iostream>     // Entrada e saída padrão (cout, cerr)
#include <fstream>      // Manipulação de arquivos (leitura e escrita binária)
#include <vector>       // Container dinâmico para armazenar bytes da imagem
#include <thread>       // Para pausar o programa com sleep_for
#include <chrono>       // Para definir duração da pausa (ex: segundos)
#include <asio.hpp>     // Biblioteca ASIO para comunicação TCP/IP
#include <windows.h>    // API do Windows para captura de tela e manipulação de imagens

using asio::ip::tcp;   // Facilita o uso do tcp::socket sem precisar especificar o namespace completo

// Função que captura a tela inteira do Windows e salva no arquivo BMP
void takeScreenshot(const std::string& filename) {
    DEVMODE screenSettings;                // Estrutura para armazenar configurações da tela
    screenSettings.dmSize = sizeof(DEVMODE);
    int screenWidth = 0, screenHeight = 0;

    // Obtem as configurações atuais da tela para saber a resolução
    if (EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &screenSettings)) {
        screenWidth = screenSettings.dmPelsWidth;   // largura da tela
        screenHeight = screenSettings.dmPelsHeight; // altura da tela
    } else {
        std::cerr << "Failed to get screen resolution." << std::endl;
        return;  // Sai da função caso não consiga pegar a resolução
    }

    // Obtém o contexto da tela para capturar pixels
    HDC hScreenDC = GetDC(NULL);
    // Cria um contexto de dispositivo de memória compatível para armazenamento temporário
    HDC hMemoryDC = CreateCompatibleDC(hScreenDC);

    // Cria um bitmap compatível com o contexto da tela, do tamanho da tela
    HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, screenWidth, screenHeight);
    // Seleciona o bitmap no contexto de memória para desenhar nele
    SelectObject(hMemoryDC, hBitmap);

    // Copia os pixels da tela para o bitmap no contexto de memória
    BitBlt(hMemoryDC, 0, 0, screenWidth, screenHeight, hScreenDC, 0, 0, SRCCOPY);

    BITMAP bmpScreen;
    // Obtém informações do bitmap criado
    GetObject(hBitmap, sizeof(BITMAP), &bmpScreen);

    BITMAPFILEHEADER bmfHeader;     // Cabeçalho do arquivo BMP
    BITMAPINFOHEADER bi;            // Cabeçalho de informações da imagem

    // Preenche o cabeçalho BITMAPINFOHEADER
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = bmpScreen.bmWidth;
    bi.biHeight = -bmpScreen.bmHeight; // Negativo para evitar imagem invertida verticalmente
    bi.biPlanes = 1;
    bi.biBitCount = 32;                // 32 bits por pixel (RGBA)
    bi.biCompression = BI_RGB;         // Sem compressão
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;

    // Calcula o tamanho da imagem em bytes (largura * 4 bytes por pixel * altura)
    DWORD dwBmpSize = bmpScreen.bmWidth * 4 * bmpScreen.bmHeight;
    // Cria buffer para armazenar os dados dos pixels
    std::vector<BYTE> bmpBuffer(dwBmpSize);

    // Copia os bits do bitmap para o buffer, para salvar no arquivo
    GetDIBits(hMemoryDC, hBitmap, 0, (UINT)bmpScreen.bmHeight, bmpBuffer.data(), (BITMAPINFO*)&bi, DIB_RGB_COLORS);

    // Abre o arquivo em modo binário para escrita
    std::ofstream file(filename, std::ios::out | std::ios::binary);

    // Preenche o cabeçalho do arquivo BMP
    bmfHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER); // Offset para os dados da imagem
    bmfHeader.bfSize = bmfHeader.bfOffBits + dwBmpSize;                      // Tamanho total do arquivo
    bmfHeader.bfType = 0x4D42;                                               // Tipo do arquivo BMP = 'BM'

    // Escreve o cabeçalho do arquivo
    file.write((char*)&bmfHeader, sizeof(BITMAPFILEHEADER));
    // Escreve o cabeçalho de informações da imagem
    file.write((char*)&bi, sizeof(BITMAPINFOHEADER));
    // Escreve os dados dos pixels
    file.write((char*)bmpBuffer.data(), dwBmpSize);

    file.close();  // Fecha o arquivo

    // Libera os recursos de GDI para evitar vazamentos de memória
    DeleteObject(hBitmap);
    DeleteDC(hMemoryDC);
    ReleaseDC(NULL, hScreenDC);
}

// Função que envia o arquivo BMP pela rede via socket TCP
void sendImage(tcp::socket& socket, const std::string& filename) {
    std::ifstream file(filename, std::ios::binary); // Abre arquivo para leitura binária

    if (!file) {
        std::cerr << "[cliente] Falha ao abrir o arquivo para envio.\n";
        return;  // Sai se não conseguir abrir o arquivo
    }

    // Move o ponteiro para o final para obter o tamanho do arquivo
    file.seekg(0, std::ios::end);
    std::size_t size = file.tellg();  // Tamanho do arquivo em bytes
    file.seekg(0, std::ios::beg);      // Volta ao início para ler

    // Cria buffer para armazenar o conteúdo do arquivo
    std::vector<char> buffer(size);
    file.read(buffer.data(), size);  // Lê o arquivo inteiro no buffer
    file.close();                    // Fecha o arquivo

    // Envia o tamanho do arquivo primeiro para o receptor saber quanto ler
    asio::write(socket, asio::buffer(&size, sizeof(size)));
    // Envia o conteúdo do arquivo (bytes da imagem)
    asio::write(socket, asio::buffer(buffer));

    std::cout << "[cliente] Imagem enviada com sucesso.\n";
}

int main() {
    try {
        asio::io_context io_context;               // Contexto de entrada/saída do ASIO
        tcp::socket socket(io_context);            // Cria socket TCP

        // Conecta ao servidor no localhost na porta 12345
        socket.connect(tcp::endpoint(asio::ip::make_address("127.0.0.1"), 12345));
        std::cout << "[cliente] Conectado ao servidor.\n";

        // Loop infinito para capturar e enviar screenshots a cada 20 segundos
        while (true) {
            takeScreenshot("print.bmp");         // Captura a tela e salva no arquivo
            sendImage(socket, "print.bmp");      // Envia a imagem para o servidor
            std::this_thread::sleep_for(std::chrono::seconds(20)); // Espera 20 segundos antes da próxima captura
        }
    } catch (std::exception& e) {
        // Em caso de erro, exibe a mensagem no terminal
        std::cerr << "[cliente] Erro: " << e.what() << std::endl;
    }

    return 0;  // Fim do programa
}
