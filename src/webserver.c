#define _CRT_SECURE_NO_WARNINGS
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <string.h>

#pragma comment(lib, "ws2_32.lib")

void serve_html(SOCKET client) {
    FILE *file = fopen("index.html", "r");
    if (!file) {
        const char *error = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\nFile not found.";
        send(client, error, strlen(error), 0);
        return;
    }

    char buffer[4096];
    const char *header = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
    send(client, header, strlen(header), 0);

    while (fgets(buffer, sizeof(buffer), file)) {
        send(client, buffer, strlen(buffer), 0);
    }

    fclose(file);
}

void handle_post(SOCKET client, const char *request) {
    const char *body = strstr(request, "\r\n\r\n");
    if (body) {
        body += 4;
        printf("Received POST data: %s\n", body);

        const char *response =
            "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
        send(client, response, strlen(response), 0);
    }
}

int main() {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    SOCKET server = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(server, (struct sockaddr*)&addr, sizeof(addr));
    listen(server, 5);
    printf("Server running at http://localhost:8080\n");

    while (1) {
        SOCKET client = accept(server, NULL, NULL);
        char buffer[8192] = {0};
        recv(client, buffer, sizeof(buffer), 0);

        if (strncmp(buffer, "POST", 4) == 0)
            handle_post(client, buffer);
        else
            serve_html(client);

        closesocket(client);
    }

    WSACleanup();
    return 0;
}