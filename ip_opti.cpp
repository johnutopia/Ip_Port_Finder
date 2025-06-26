#include <iostream>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <fstream>
#include <thread>
#include <mutex>
#include <vector>
#include <atomic>

std::mutex outputMutex;
std::ofstream output;
std::atomic<int> scannedCount(0);
int totalPorts = 0;

void scanPort(const std::string& ip, int port) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::lock_guard<std::mutex> lock(outputMutex);
        std::cerr << "❌ Socket creation failed for port " << port << std::endl;
        return;
    }

    sockaddr_in target{};
    target.sin_family = AF_INET;
    target.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &target.sin_addr);

    // Non-blocking socket for timeout
    fcntl(sock, F_SETFL, O_NONBLOCK);

    int connectResult = connect(sock, (sockaddr*)&target, sizeof(target));
    if (connectResult < 0) {
        fd_set fdset;
        struct timeval tv;

        FD_ZERO(&fdset);
        FD_SET(sock, &fdset);
        tv.tv_sec = 0;
        tv.tv_usec = 300000; // 300 ms timeout

        if (select(sock + 1, NULL, &fdset, NULL, &tv) == 1) {
            int so_error;
            socklen_t len = sizeof so_error;
            getsockopt(sock, SOL_SOCKET, SO_ERROR, &so_error, &len);

            std::lock_guard<std::mutex> lock(outputMutex);
            if (so_error == 0) {
                std::cout << "✅ Port " << port << " is OPEN\n";
                output << "Port " << port << ": OPEN\n";
            } else {
                std::cout << "❌ Port " << port << " is CLOSED/FILTERED\n";
                output << "Port " << port << ": CLOSED/FILTERED\n";
            }
        } else {
            std::lock_guard<std::mutex> lock(outputMutex);
            std::cout << "❌ Port " << port << " is CLOSED/FILTERED (timeout)\n";
            output << "Port " << port << ": CLOSED/FILTERED (timeout)\n";
        }
    } else {
        std::lock_guard<std::mutex> lock(outputMutex);  
        std::cout << "✅ Port " << port << " is OPEN\n";
        output << "Port " << port << ": OPEN\n";
    }

    close(sock);

    scannedCount++; 
    // Optional: print progress
    {
        std::lock_guard<std::mutex> lock(outputMutex);
        std::cout << "[Progress] Scanned " << scannedCount << " / " << totalPorts << " ports\r" << std::flush;
    }
}

void scanPortsMultithreaded(const std::string& ip, int startPort, int endPort) {
    output.open("port_scan_results.txt");
    std::cout << "\n🔍 Starting port scan on " << ip << " from port " << startPort << " to " << endPort << "...\n"; 

    totalPorts = endPort - startPort + 1;
    scannedCount = 0;

    std::vector<std::thread> threads;
    const int maxThreads = 100;
    int activeThreads = 0;

    for (int port = startPort; port <= endPort; ++port) {
        threads.emplace_back(scanPort, ip, port);
        activeThreads++;

        if (activeThreads >= maxThreads) {
            // Wait for current batch to finish
            for (auto& t : threads) {
                if (t.joinable())
                    t.join();
            }
            threads.clear();
            activeThreads = 0;
        }
    }

    // Join remaining threads
    for (auto& t : threads) {
        if (t.joinable())
            t.join();
    }

    output.close();
    std::cout << "\n\n📝 Port scan results saved to port_scan_results.txt\n";
}

void getDomainIP(const std::string& domain, std::string& ip) {
    struct hostent* host = gethostbyname(domain.c_str());
    if (host == NULL) {
        std::cerr << "❌ Failed to resolve domain!" << std::endl;
        ip = "";
        return;
    }

    struct in_addr* addr = (struct in_addr*)host->h_addr_list[0];
    ip = inet_ntoa(*addr);
    std::cout << "🌐 IP Address of " << domain << ": " << ip << std::endl;
}

void getLocalIP() {
    char hostbuffer[256];
    char* IPbuffer;
    struct hostent* host_entry;

    if (gethostname(hostbuffer, sizeof(hostbuffer)) == -1) {
        std::cerr << "❌ Failed to get hostname!" << std::endl;
        return;
    }

    host_entry = gethostbyname(hostbuffer);
    if (host_entry == NULL) {
        std::cerr << "❌ Failed to get host entry!" << std::endl;
        return;
    }

    IPbuffer = inet_ntoa(*((struct in_addr*)host_entry->h_addr_list[0]));
    std::cout << "💻 Local Hostname: " << hostbuffer << std::endl;
    std::cout << "📡 Local IP Address: " << IPbuffer << std::endl;
}

void pingTarget(const std::string& target) {
    std::string command = "ping -c 4 " + target;
    std::cout << "📶 Pinging " << target << "...\n\n";
    system(command.c_str());
}

int main() {
    std::string domain, ip;
    std::cout << "🔍 Enter a domain to resolve (e.g. google.com): ";
    std::cin >> domain;

    getDomainIP(domain, ip);
    if (ip.empty()) {
        std::cerr << "Exiting due to domain resolution failure." << std::endl;
        return 1;
    }

    std::cout << std::endl;

    getLocalIP();
    std::cout << std::endl;

    char choice;
    std::cout << "🔁 Want to ping this domain? (y/n): ";
    std::cin >> choice;

    if (choice == 'y' || choice == 'Y') {
        pingTarget(domain);
    }

    std::cout << std::endl;

    std::cout << "📦 Want to scan open ports? (y/n): ";
    std::cin >> choice;

    if (choice == 'y' || choice == 'Y') {
        int startPort, endPort;
        std::cout << "🛠 Enter start port (e.g. 1): ";
        std::cin >> startPort;
        std::cout << "🛠 Enter end port (e.g. 1024): ";
        std::cin >> endPort;

        if (startPort > 0 && endPort >= startPort && endPort <= 65535) {
            scanPortsMultithreaded(ip, startPort, endPort);
        } else {
            std::cerr << "❌ Invalid port range." << std::endl;
        }
    }

    std::cout << "\n✅ Program finished. Have a good hacking day! 🚀\n";
    return 0;
}
