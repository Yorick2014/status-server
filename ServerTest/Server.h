#pragma once

#include <vector>
#include <thread>

class HttpServer {
public:
    HttpServer();
    ~HttpServer();
    void runSync();
private:
    std::vector<std::thread> threads;
};
