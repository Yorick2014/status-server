﻿#include <iostream>
#include <WinSock2.h>
#include <ctime>
#include <thread>
#include <vector>
#include <mutex>
#include <string>


#pragma comment (lib, "Ws2_32.lib")
#pragma warning(disable: 4996)

void send_content(SOCKET client_socket) {

    std::time_t now = std::time(0);
    char* dt = std::ctime(&now);        

    // HTTP-ответ с датой и временем
    std::string body = "<html><body><h1>Current Date and Time</h1><p>" + std::string(dt) + "</p></body></html>";
    std::string response =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: " + std::to_string(body.size()) +"\r\n"
        "Connection: close\r\n\r\n"
        +body;

    
    // Отправляем ответ клиенту
    //std::cout << "socket: " + client_socket << "\n";
    send(client_socket, response.c_str(), response.length(), 0);
    
}

// Функция для обработки нового соединения
void handle_requests(SOCKET client_socket, int counts) {


}

int main() {
    WSAData wsaData;
    WORD DLLVersion = MAKEWORD(2, 1);
    if (WSAStartup(DLLVersion, &wsaData) != 0) {
        std::cout << "Error init WSA\n";
        exit(1);
    }

    SOCKADDR_IN addr;
    int sizeofaddr = sizeof(addr);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(15476);
    addr.sin_family = AF_INET;

    SOCKET sListen = socket(AF_INET, SOCK_STREAM, NULL);
    if (bind(sListen, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        std::cout << "Bind failed with error: " << WSAGetLastError() << "\n";
        closesocket(sListen);
        WSACleanup();
        return 1;
    }
    std::cout << "Socket addr: " << (SOCKADDR*)&addr << std::endl;
    if (listen(sListen, SOMAXCONN) == SOCKET_ERROR) { // прослушивание
        std::cout << "Listen failed with error: " << WSAGetLastError() << "\n";
        closesocket(sListen);
        WSACleanup();
        return 1;
    }

    std::thread threads; // Для хранения потоков
    //std::vector Thr = threads;

    std::cout << "Waiting for client connections...\n";
    int count_req = 0;
    char buffer[1024];

    
    while (true) {

        SOCKET newConnection = accept(sListen, (SOCKADDR*)&addr, &sizeofaddr); //(SOCKADDR*)&addr IP клиента
        if (newConnection == INVALID_SOCKET) {
            std::cout << "Failed to accept connection\n";
            continue; // Пропускаем итерацию, если не удалось принять соединение
        }

        std::cout << "\nID req " << count_req << std::endl;

        int bytesReceived = recv(newConnection, buffer, sizeof(buffer) - 1, 0);
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';


            //std::cout << "\nSocket:" << std::to_string(sock_info) << "\n";
            std::cout << "Received request:\n" << buffer << std::endl;

            // Отправляем ответ клиенту
            //send_content(newConnection);

        }
        else if (bytesReceived == 0) {
            // Клиент закрыл соединение
            std::cout << "Client disconnected.\n";
            //for (std::thread& t : threads) {
            //    if (t.joinable()) {
            //        t.join();
            //    }
            //}
        }
        else {
            // Произошла ошибка
            std::cout << "Failed to receive request\n";
        }

        // Создание потока для обработки клиента        
        
        
        threads.emplace_back(send_content, newConnection);
        count_req++;
    }

    // Ожидание завершения всех потоков
    std::cout << "\n END \n";


    closesocket(sListen);
    WSACleanup();
    return 0;
}
