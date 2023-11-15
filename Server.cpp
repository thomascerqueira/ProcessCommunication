#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <map>
#include <functional>
#include <csignal>

// Fonction pour gérer un message spécifique
void handleHello(const std::string &message)
{
    std::cout << "Message 'Hello' reçu : " << message << std::endl;
}

// Fonction pour gérer un message spécifique
void handleGoodbye(const std::string &message)
{
    std::cout << "Message 'Goodbye' reçu : " << message << std::endl;
}

class Server
{
private:
    int server_socket;
    Server(/* args */);
    ~Server();

    Server(Server const &) = delete;
    void operator=(Server const &) = delete;

    std::map<std::string, std::function<void(const std::string &)>> messageHandlers = {
        {"Hello", handleHello},
        {"Goodbye", handleGoodbye},
    };

    bool continueRunning = true;
    std::string pathMessage = "/tmp/mon_service_socket";

public:
    static Server &getInstance()
    {
        static Server server{};
        return server;
    }

    int getSocket()
    {
        return server_socket;
    }

    void stop()
    {
        continueRunning = false;
    }

    int run();
};

sig_atomic_t signal_received = 0;

void signalHandler(int signal)
{
    std::cout << "Signal reçu : " << signal << std::endl;
    signal_received = signal;
    std::cout << "Signal traité." << std::endl;
}

Server::Server(/* args */)
{
    std::cout << "construct" << std::endl;
    std::cout << "Création du socket..." << std::endl;
    server_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    std::cout << "Socket créé. " << server_socket << std::endl;

    if (server_socket == -1)
    {
        throw std::runtime_error("Erreur lors de la création du socket.");
    }

    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, NULL, 0);

    sockaddr_un server_address;
    server_address.sun_family = AF_UNIX;
    strcpy(server_address.sun_path, pathMessage.c_str());

    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1)
    {
        throw std::runtime_error("Erreur lors du bind du socket.");
    }

    if (listen(server_socket, 5) == -1)
    {
        throw std::runtime_error("Erreur lors de l'écoute du socket.");
    }
}

int Server::run()
{
    int client_socket = accept(server_socket, NULL, NULL);
    if (client_socket == -1)
    {
        return -1;
    }

    std::string message;
    char buffer[256];
    ssize_t bytes_received;
    while ((bytes_received = recv(client_socket, buffer, sizeof(buffer), 0)) > 0)
    {
        buffer[bytes_received] = '\0';
        message += buffer;
    }

    if (bytes_received == -1)
    {
        return -1;
    }

    if (messageHandlers.find(message) != messageHandlers.end())
    {
        messageHandlers[message](message);
    }
    else
    {
        std::cout << "Message non reconnu : " << message << std::endl;
    }

    close(client_socket);
    return 0;
}

Server::~Server()
{
    std::cout << "destruct" << std::endl;
    std::remove(pathMessage.c_str());
    close(server_socket);
}

int main()
{
    struct sigaction sigact;

    sigemptyset(&sigact.sa_mask);
    sigact.sa_flags = 0;
    sigact.sa_handler = signalHandler;
    sigaction(SIGINT, &sigact, NULL);

    Server &server = Server::getInstance();

    while (server.run() != 1 && signal_received == 0)
    {
    }
    return 0;
}