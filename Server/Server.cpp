#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <winsock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <string>
#include <vector>

#pragma comment(lib, "Ws2_32.lib")

using namespace std;

struct Client {
    SOCKET sock;
    string name;
};

vector<Client> clients;

void Broadcast(string msg, SOCKET from = INVALID_SOCKET) {
    for (auto& c : clients) {
        if (c.sock != from) {
            send(c.sock, msg.c_str(), msg.length(), 0);
        }
    }
}

string GetUserList() {
    string list = "Users online: ";
    for (size_t i = 0; i < clients.size(); i++) {
        if (i > 0) list += ", ";
        list += clients[i].name;
    }
    return list;
}

DWORD WINAPI HandleClient(LPVOID param) {
    SOCKET clientSock = (SOCKET)param;
    char buf[512];
    string name;

    // Получаем имя
    int bytes = recv(clientSock, buf, 512, 0);
    if (bytes > 0) {
        buf[bytes] = '\0';
        name = buf;

        clients.push_back({ clientSock, name });

        string msg = "[SERVER]: user \"" + name + "\" has joined";
        cout << msg << endl;
        Broadcast(msg);

        send(clientSock, "Welcome to chat! Type /help for commands\n", 41, 0);
    }

    // Основной цикл
    while (true) {
        bytes = recv(clientSock, buf, 512, 0);

        if (bytes <= 0) {
            // Отключение клиента
            for (size_t i = 0; i < clients.size(); i++) {
                if (clients[i].sock == clientSock) {
                    string msg = "[SERVER]: user \"" + clients[i].name + "\" left the chat";
                    cout << msg << endl;
                    Broadcast(msg);
                    clients.erase(clients.begin() + i);
                    break;
                }
            }
            closesocket(clientSock);
            return 0;
        }

        buf[bytes] = '\0';
        string msg = buf;
        if (!msg.empty() && msg.back() == '\n') msg.pop_back();

        // Команды
        if (msg == "/users") {
            string list = GetUserList() + "\n";
            send(clientSock, list.c_str(), list.length(), 0);
        }
        else if (msg == "/help") {
            string help = "Commands:\n/users - show users\n/exit - leave chat\n";
            send(clientSock, help.c_str(), help.length(), 0);
        }
        else if (msg == "/exit") {
            break;
        }
        else {
            // Находим имя отправителя
            string sender;
            for (auto& c : clients) {
                if (c.sock == clientSock) {
                    sender = c.name;
                    break;
                }
            }

            string formatted = "[" + sender + "]: " + msg;
            cout << formatted << endl;
            Broadcast(formatted, clientSock);
        }
    }

    return 0;
}

int main() {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    SOCKET listenSock = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(27010);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(listenSock, (sockaddr*)&addr, sizeof(addr));
    listen(listenSock, SOMAXCONN);

    while (true) {
        SOCKET clientSock = accept(listenSock, NULL, NULL);
        cout << "New client connected!" << endl;

        send(clientSock, "Enter your name: ", 18, 0);

        CreateThread(NULL, 0, HandleClient, (LPVOID)clientSock, 0, NULL);
    }

    closesocket(listenSock);
    WSACleanup();
    return 0;
}