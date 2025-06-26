# 🔍 C++ Multithreaded Port Scanner & Network Utility

A fast, minimal, and multithreaded terminal-based port scanner and network diagnostic utility built with C++. This tool helps you:
- Resolve a domain name to an IP
- Ping a domain
- Retrieve your local IP
- Perform a multithreaded TCP port scan

## 🚀 Features

- 🌐 Resolve domain to IP
- 📡 Detect local IP address
- 📶 Ping remote hosts
- 🔎 Scan TCP ports using multithreading
- 🧵 Customizable thread limit
- 📝 Outputs scan results to `port_scan_results.txt`
- ⏱️ 300ms timeout per port for accurate scanning

---

## 📦 Dependencies

This program uses standard Linux/C++ libraries:
- `<iostream>`, `<fstream>`, `<thread>`, `<mutex>`, `<vector>`, `<atomic>`
- `<netdb.h>`, `<arpa/inet.h>`, `<unistd.h>`, `<sys/socket.h>`, `<netinet/in.h>`, `<fcntl.h>`

### ✅ Works on:
- Linux
- WSL (Windows Subsystem for Linux)
- macOS (with slight modification)

---
## 📄 License

This project is licensed under the [MIT License](LICENSE).


## 🧪 How to Compile

Make sure you have `g++` installed:

```bash
g++ -std=c++11 port_scanner.cpp -o port_scanner -pthread
