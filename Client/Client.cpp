#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <winsock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <string>
#include <thread>

#pragma comment(lib, "Ws2_32.lib")

using namespace std;

SOCKET sock;
bool connected = true;

void ReceiveMessages() {
    char buf[512];
    while (connected) {
        int bytes = recv(sock, buf, 512, 0);
        if (bytes > 0) {
            buf[bytes] = '\0';
            cout << "\r" << buf << "\n> ";
        }
        else {
            connected = false;
            cout << "\nConnection lost!" << endl;
            break;
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cout << "Usage: " << argv[0] << " <server_ip>" << endl;
        return 1;
    }

    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    sock = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(27010);
    inet_pton(AF_INET, argv[1], &addr.sin_addr);

    if (connect(sock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        cout << "Failed to connect!" << endl;
        return 1;
    }

    cout << "Connected to server!" << endl;

    // Ввод сообщения
    string name;
    cout << "Enter your messange: ";
    getline(cin, name);
    send(sock, name.c_str(), name.length(), 0);

    // Запускаем прием сообщений
    thread receiver(ReceiveMessages);
    receiver.detach();

    // Цикл отправки
    string msg;
    while (connected) {
        cout << "> ";
        getline(cin, msg);

        if (msg == "/exit") {
            send(sock, "/exit", 5, 0);
            break;
        }

        send(sock, msg.c_str(), msg.length(), 0);
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}