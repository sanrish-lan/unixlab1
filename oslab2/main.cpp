#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <csignal>
#include <cstring>
#include <cerrno>
#include <iostream>

#define SERVER_PORT 7373
#define BUFFER_SIZE 1024

volatile sig_atomic_t signal_received = 0;

void signal_handler(int signal_number)
{
    if (signal_number == SIGHUP)
    {
        signal_received = 1;
    }
}

class NetworkServer
{
private:
    int server_fd;
    int client_fd;

public:
    NetworkServer() : server_fd(-1), client_fd(-1) {}

    bool initialize()
    {
        server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd < 0)
        {
            std::cerr << "Socket creation error" << std::endl;
            return false;
        }


        int opt_value = 1;
        setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt_value, sizeof(opt_value));

        struct sockaddr_in server_addr;
        std::memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(SERVER_PORT);
        server_addr.sin_addr.s_addr = INADDR_ANY;

        if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
        {
            std::cerr << "Socket binding error" << std::endl;
            return false;
        }

        if (listen(server_fd, 1) < 0)
        {
            std::cerr << "Listen error" << std::endl;
            return false;
        }

        return true;
    }

    void handle_new_connection()
    {
        if (client_fd != -1)
        {
            std::cout << "Existing connection is active, rejecting new one" << std::endl;
            int temp_socket = accept(server_fd, nullptr, nullptr);
            if (temp_socket >= 0) close(temp_socket);
            return;
        }

        client_fd = accept(server_fd, nullptr, nullptr);
        if (client_fd >= 0)
        {
            std::cout << "New connection established: descriptor " << client_fd << std::endl;
        }
    }

    void process_client_data()
    {
        char data_buffer[BUFFER_SIZE];
        int bytes_received = read(client_fd, data_buffer, sizeof(data_buffer));

        if (bytes_received > 0)
        {
            std::cout << "Data received (bytes): " << bytes_received << std::endl;
        }
        else
        {
            std::cout << "Client disconnected" << std::endl;
            close(client_fd);
            client_fd = -1;
        }
    }

    int get_max_descriptor() const
    {
        int max_desc = server_fd;
        if (client_fd > max_desc) max_desc = client_fd;
        return max_desc;
    }

    void prepare_descriptor_set(fd_set& read_set) const
    {
        FD_ZERO(&read_set);
        FD_SET(server_fd, &read_set);
        if (client_fd != -1)
        {
            FD_SET(client_fd, &read_set);
        }
    }

    bool is_server_ready(const fd_set& read_set) const
    {
        return FD_ISSET(server_fd, &read_set);
    }

    bool is_client_ready(const fd_set& read_set) const
    {
        return (client_fd != -1) && FD_ISSET(client_fd, &read_set);
    }

    void cleanup()
    {
        if (client_fd != -1) close(client_fd);
        if (server_fd != -1) close(server_fd);
    }

    int get_server_fd() const { return server_fd; }
    int get_client_fd() const { return client_fd; }
};

void setup_signal_handling(sigset_t& original_mask)
{
    sigset_t blocked_set;
    sigemptyset(&blocked_set);
    sigaddset(&blocked_set, SIGHUP);
    sigprocmask(SIG_BLOCK, &blocked_set, &original_mask);

    struct sigaction sa;
    std::memset(&sa, 0, sizeof(sa));
    sa.sa_handler = signal_handler;
    sa.sa_flags = 0;
    sigaction(SIGHUP, &sa, nullptr);
}

int main()
{
    NetworkServer server;

    if (!server.initialize())
    {
        return EXIT_FAILURE;
    }

    std::cout << "Server started. Port: " << SERVER_PORT  << ", Process ID: " << getpid() << std::endl;
    std::cout << "To send SIGHUP signal use: kill -HUP " << getpid() << std::endl;

    sigset_t original_signal_mask;
    setup_signal_handling(original_signal_mask);

    while (true)
    {
        fd_set read_descriptors;
        server.prepare_descriptor_set(read_descriptors);

        int max_fd = server.get_max_descriptor();
        int ready_count = pselect(max_fd + 1, &read_descriptors, nullptr, nullptr, nullptr, &original_signal_mask);

        if (ready_count < 0)
        {
            if (errno == EINTR)
            {
                if (signal_received)
                {
                    std::cout << "[SIGNAL] Received SIGHUP signal" << std::endl;
                    signal_received = 0;
                }
                continue;
            }
            std::cerr << "pselect error" << std::endl;
            break;
        }

        if (signal_received)
        {
            std::cout << "[SIGNAL] Signal flag is set" << std::endl;
            signal_received = 0;
        }

        if (server.is_server_ready(read_descriptors))
        {
            server.handle_new_connection();
        }

        if (server.is_client_ready(read_descriptors))
        {
            server.process_client_data();
        }
    }

    server.cleanup();
    return 0;
}