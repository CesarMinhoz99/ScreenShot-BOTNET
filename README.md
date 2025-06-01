# Botnet Screenshot

Exemplo básico de comunicação cliente-servidor em C++ usando **ASIO**, onde o cliente (simulando um bot infectado) captura a tela do computador a cada 20 segundos e envia a imagem para o servidor. O servidor recebe e salva os prints localmente.

---

## Funcionalidades

- Cliente captura screenshot da tela do Windows usando API Win32.  
- Cliente envia a imagem via socket TCP usando ASIO.  
- Servidor aceita conexões, recebe imagens e salva com nomes sequenciais.  
- Comunicação simples para fins educacionais.

---

## Pré-requisitos

- Windows (captura de tela usa API Win32).  
- Compilador com suporte a C++17.  
- Biblioteca ASIO (standalone).  
- Bibliotecas do Windows: ws2_32, gdi32.

---

## Onde baixar ASIO

Você pode baixar a biblioteca ASIO standalone em:

- Repositório oficial GitHub: [https://github.com/chriskohlhoff/asio](https://github.com/chriskohlhoff/asio)  
- Versão estável no SourceForge: [https://sourceforge.net/projects/asio/files/asio/1.30.2%20%28Stable%29/](https://sourceforge.net/projects/asio/files/asio/1.30.2%20%28Stable%29/)

Passos para usar a versão baixada:

1. Baixe o arquivo ZIP da versão estável (exemplo: 1.30.2).  
2. Extraia o conteúdo para uma pasta no seu computador.  
3. Use o caminho para a pasta `asio/include` no comando de compilação, por exemplo:  
   `-IC:/caminho_para_asio/include`

---

## Como compilar

Exemplo usando g++ (MinGW):

```bash
g++ -std=c++17 -IC:/path_to_asio/include client.cpp -o client -lws2_32 -lgdi32
g++ -std=c++17 -IC:/path_to_asio/include server.cpp -o server -lws2_32
