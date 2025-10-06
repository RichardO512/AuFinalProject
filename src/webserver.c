// Disable warnings for older C functions like fopen and sprintf
#define _CRT_SECURE_NO_WARNINGS

// Include networking and standard libraries
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <string.h>

// Link the Windows socket library
#pragma comment(lib, "ws2_32.lib")

// Serve a file to the client (like index.html or style.css)
void serve_file(SOCKET client, const char *path) {
    FILE *file = fopen(path, "rb"); // Open file in binary mode

    if (!file) {
        // If no file  send a 404 error
        const char *error = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\nFile not found.";
        send(client, error, strlen(error), 0);
        return;
    }

    // Determine the file type based on its extension
    const char *ext = strrchr(path, '.'); // Find last dot in filename
    const char *type = "text/plain";      // Default type

    if (ext) {
        if (strcmp(ext, ".html") == 0) type = "text/html";
        else if (strcmp(ext, ".css") == 0) type = "text/css";
        else if (strcmp(ext, ".js") == 0) type = "application/javascript";
        else if (strcmp(ext, ".png") == 0) type = "image/png";
        else if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0) type = "image/jpeg";
    }

    // Send HTTP header with correct content type
    char header[256];
    sprintf(header, "HTTP/1.1 200 OK\r\nContent-Type: %s\r\n\r\n", type);
    send(client, header, strlen(header), 0);

    // Send the file content in chunks
    char buffer[4096];
    size_t bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        send(client, buffer, bytes, 0);
    }

    fclose(file); // Close the file
}

// Handle GET requests and route to the correct file
void route_request(SOCKET client, const char *request) {
    char path[512] = {0};

    // Extract the requested file path from the GET request
    sscanf(request, "GET /%s ", path);

    // If no specific file is requested, default to index.html
    if (strlen(path) == 0 || strcmp(path, "/") == 0 || strstr(request, "GET / HTTP") != NULL) {
        strcpy(path, "index.html");
    }

    // Decode %20 (space) in URLs
    for (int i = 0; path[i]; i++) {
        if (strncmp(&path[i], "%20", 3) == 0) {
            path[i] = ' ';
            memmove(&path[i + 1], &path[i + 3], strlen(&path[i + 3]) + 1);
        }
    }

    // Only allow specific files and folders to be served
    if (
        strcmp(path, "index.html") == 0 ||
        strcmp(path, "form.html") == 0 ||
        strcmp(path, "about.html") == 0 ||
        strstr(path, "css/") == path ||
        strstr(path, "js/") == path ||
        strstr(path, "img/") == path
    ) {
        serve_file(client, path); // Serve the requested file
    } else {
        // If the file is not allowed, send a 404 error
        const char *error = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\nPage not allowed.";
        send(client, error, strlen(error), 0);
    }
}

// Handle POST requests (like form submissions)
void handle_post(SOCKET client, const char *request) {
    const char *body = strstr(request, "\r\n\r\n"); // Find start of POST body
    if (body) {
        body += 4; // Skip the header separator
        printf("📥 Received POST data:\n%s\n", body); // Print data to console

        // Send HTML with modal and redirect after 2 seconds
        const char *response =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n\r\n"
            "<!DOCTYPE html>"
            "<html lang=\"en\">"
            "<head>"
            "<meta charset=\"UTF-8\">"
            "<title>Redirecting...</title>"
            "<style>"
            "#modal {"
            "  position: fixed;"
            "  top: 50%;"
            "  left: 50%;"
            "  transform: translate(-50%, -50%);"
            "  background-color: white;"
            "  padding: 20px;"
            "  box-shadow: 0 5px 15px rgba(0,0,0,0.3);"
            "  border-radius: 8px;"
            "  text-align: center;"
            "  font-family: Arial, sans-serif;"
            "}"
            "</style>"
            "</head>"
            "<body>"
            "<div id=\"modal\">"
            "<h2>✅ Successfully Enrolled!</h2>"
            "<p>Please wait for the university admin to contact you.</p>"
            "</div>"
            "<script>"
            "setTimeout(() => {"
            "  document.getElementById('modal').style.display = 'none';"
            "  window.location.href = '/index.html';"
            "}, 2000);"
            "</script>"
            "</body>"
            "</html>";

        send(client, response, strlen(response), 0); // Send the response
    }
}

// Main function: starts the server
int main() {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa); // Start Windows socket system

    SOCKET server = socket(AF_INET, SOCK_STREAM, 0); // Create TCP socket
    struct sockaddr_in addr = {0}; // Server address setup
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080); // Listen on port 8080
    addr.sin_addr.s_addr = INADDR_ANY; // Accept connections from any IP

    bind(server, (struct sockaddr*)&addr, sizeof(addr)); // Bind socket
    listen(server, 5); // Start listening for connections
    printf("✅ Server running at http://localhost:8080\n");

    // Main loop: accept and handle client requests
    while (1) {
        SOCKET client = accept(server, NULL, NULL); // Accept connection
        char buffer[8192] = {0}; // Buffer to store request
        recv(client, buffer, sizeof(buffer), 0); // Receive request

        // Handle POST or GET requests
        if (strncmp(buffer, "POST", 4) == 0 && strstr(buffer, "POST /submit") != NULL)
            handle_post(client, buffer);
        else if (strncmp(buffer, "GET", 3) == 0)
            route_request(client, buffer);

        closesocket(client); // Close client connection
    }

    WSACleanup(); // Clean up Windows socket system
    return 0;
}