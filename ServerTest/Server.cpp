﻿#include <iostream>
#include "Server.h"
#include <vector>
#include <thread>
#include <string>
#include <WinSock2.h>

#include <ctime>
#include <mutex>

#pragma comment (lib, "Ws2_32.lib")
#pragma warning(disable: 4996)

HttpServer::HttpServer() {
}

HttpServer::~HttpServer() {
}

class Threads 
{
    std::vector<std::thread> threads;

public:
    bool check_thread = false;

    void close_threads() {

        for (std::thread& t : threads) {
            if (t.joinable()) {
                t.join();
            }
        }
        check_thread = false; //мьютекс
    }
    
    void socket_number(SOCKADDR addr)
    {
        std::cout << "Socket addr: " << (SOCKADDR*)&addr << std::endl;
    }

    void send_content(SOCKET client_socket) {
        std::time_t now = std::time(0);
        char* dt = std::ctime(&now);

        // HTTP-ответ с датой и временем
        std::string body = "<html><body><h1>Current Date and Time</h1><p>" + std::string(dt) + "</p></body></html>";
        std::string response =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: " + std::to_string(body.size()) + "\r\n"
            "Connection: close\r\n\r\n"
            + body;

        // Отправляем ответ клиенту
        send(client_socket, response.c_str(), response.length(), 0);
        check_thread = false; //мьютекс
    }

    // Функция для обработки нового соединения
    void handle_requests(SOCKET client_socket, int counts) {
        if (check_thread == true)
        {
            close_threads();
        }
        
        check_thread = true; //мьютекс

        char buffer[1024];
        int len = 10;
                
        int bytesReceived = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';

            //std::cout << "\nSocket:" << std::to_string(sock_info) << "\n";
            std::cout << "Received request:\n" << buffer << std::endl;

            // Отправляем ответ клиенту
            send_content(client_socket);
        }
        else if (bytesReceived == 0) {
            std::cout << "Client disconnected.\n";
        }
        else {
            std::cout << "Failed to receive request\n";
        }
    }
};

void HttpServer::runSync()
{
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

    SOCKET sockListen = socket(AF_INET, SOCK_STREAM, NULL);
    if (bind(sockListen, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        std::cout << "Bind failed with error: " << WSAGetLastError() << "\n";
        closesocket(sockListen);
        WSACleanup();
    }

    if (listen(sockListen, SOMAXCONN) == SOCKET_ERROR) { // прослушивание
        std::cout << "Listen failed with error: " << WSAGetLastError() << "\n";
        closesocket(sockListen);
        WSACleanup();
    }

    Threads threads;

    std::cout << "Waiting for client connections...\n";
    int count_req = 0;

    while (true) {

        SOCKET newConnection = accept(sockListen, (SOCKADDR*)&addr, &sizeofaddr); //(SOCKADDR*)&addr IP клиента
        if (newConnection == INVALID_SOCKET) {
            std::cout << "Failed to accept connection\n";
            continue; // Пропускаем итерацию, если не удалось принять соединение
        }

        // Создание потока для обработки клиента 
        threads.handle_requests(newConnection, count_req);
        if (threads.check_thread == false)
        {
            std::cout << "check_thread == false" << std::endl;

        }
        else {
            std::cout << "check_thread != false" << std::endl;
        }

        count_req++;
    }

    closesocket(sockListen);
    WSACleanup();
}
