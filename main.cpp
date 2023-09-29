#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <vector>
#include <chrono>
#include <winsock.h>
#pragma comment(lib, "ws2_32.lib")

std::mutex mtx;

void scanPort(const std::string& ip, int port) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::lock_guard<std::mutex> lock(mtx);
        std::cerr << "Failed to initialize WinSock" << std::endl;
        return;
    }

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        std::lock_guard<std::mutex> lock(mtx);
        std::cerr << "Failed to create socket" << std::endl;
        WSACleanup();
        return;
    }

    sockaddr_in server;
    server.sin_addr.s_addr = inet_addr(ip.c_str());
    server.sin_family = AF_INET;
    server.sin_port = htons(port);

    if (connect(sock, reinterpret_cast<struct sockaddr*>(&server), sizeof(server)) == SOCKET_ERROR) {
        std::lock_guard<std::mutex> lock(mtx);
        std::cout << "Port " << port << " is closed" << std::endl;
    }
    else {
        std::lock_guard<std::mutex> lock(mtx);
        std::cout << "Port " << port << " is open" << std::endl;
        closesocket(sock);
    }

    WSACleanup();
}

int main() {
    std::string ip;
    int startPort, endPort, numThreads;

    std::cout << "Enter the IP address to scan: ";
    std::cin >> ip;

    std::cout << "Enter the starting port number: ";
    std::cin >> startPort;

    std::cout << "Enter the ending port number: ";
    std::cin >> endPort;

    std::cout << "Enter the number of threads: ";
    std::cin >> numThreads;

    std::vector<std::thread> threads;

    auto startTime = std::chrono::high_resolution_clock::now();

    int numPorts = endPort - startPort + 1;
    int portsPerThread = numPorts / numThreads;
    int extraPorts = numPorts % numThreads;

    int currentPort = startPort;

    for (int i = 0; i < numThreads; ++i) {
        int threadPorts = portsPerThread + (i < extraPorts ? 1 : 0);

        std::thread t([&ip, currentPort, threadPorts]() {
            for (int j = 0; j < threadPorts; ++j) {
                scanPort(ip, currentPort + j);
            }
            });

        threads.push_back(std::move(t));
        currentPort += threadPorts;
    }

    for (auto& t : threads) {
        t.join();
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime);

    std::cout << "Scanning completed in " << duration.count() << " seconds" << std::endl;

    return 0;
}