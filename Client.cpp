#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <string>

int main() {
    int client_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client_socket == -1) {
        std::cerr << "Erreur lors de la création du socket." << std::endl;
        return 1;
    }
    std::string pathMessage = "/tmp/mon_service_socket";

    sockaddr_un server_address;
    server_address.sun_family = AF_UNIX;
    strcpy(server_address.sun_path, pathMessage.c_str());

    if (connect(client_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) {
        std::cerr << "Erreur lors de la connexion au service" << std::endl;
        return 1;
    }

    const char* message = "Bonjour, cest moi tchoupi !";
    send(client_socket, message, strlen(message), 0);

    close(client_socket);

    return 0;
}