#include "manager/ipcServer.hpp"
#include <sys/socket.h>
#include <sys/un.h>

#include <unistd.h>
#include <cstring>
#include <iostream>

namespace cc::manager {

IpcServer::IpcServer(std::string socketPath)
    : socketPath_(std::move(socketPath))
{
}

IpcServer::~IpcServer()
{
    stop();
}

bool IpcServer::start()
{
    unlink(socketPath_.c_str());

    serverFd_ = socket(AF_UNIX, SOCK_STREAM, 0);

    if (serverFd_ < 0) {
        perror("socket");
        return false;
    }

    sockaddr_un addr{};
    addr.sun_family = AF_UNIX;

    std::strncpy(
        addr.sun_path,
        socketPath_.c_str(),
        sizeof(addr.sun_path) - 1);

    if (bind(
            serverFd_,
            reinterpret_cast<sockaddr*>(&addr),
            sizeof(addr)) < 0) {
        perror("bind");
        return false;
    }

    if (listen(serverFd_, 5) < 0) {
        perror("listen");
        return false;
    }

    running_ = true;

    acceptThread_ =
        std::thread(&IpcServer::acceptLoop, this);

    return true;
}

void IpcServer::stop()
{
    running_ = false;

    if (serverFd_ >= 0) {
        close(serverFd_);
        serverFd_ = -1;
    }

    if (acceptThread_.joinable()) {
        acceptThread_.join();
    }

    unlink(socketPath_.c_str());
}

void IpcServer::acceptLoop()
{
    while (running_) {

        int clientFd = accept(serverFd_, nullptr, nullptr);

        if (clientFd < 0) {
            continue;
        }

        std::lock_guard lock(clientsMutex_);

        clients_.push_back(clientFd);

        std::cout
            << "IPC client connected"
            << std::endl;
    }
}

void IpcServer::closeClient(int clientFd)
{
    close(clientFd);

    for (auto it = clients_.begin(); it != clients_.end(); ++it) {
        if (*it == clientFd) {
            clients_.erase(it);
            break;
        }
    }
}

bool IpcServer::broadcastLine(const std::string& line)
{
    std::lock_guard<std::mutex> lock(clientsMutex_);

    for (size_t i = 0; i < clients_.size(); ) {

        int clientFd = clients_[i];

        ssize_t ret = send(
            clientFd,
            line.c_str(),
            line.size(),
            0);

        if (ret < 0) {
            close(clientFd);
            clients_.erase(clients_.begin() + i);
        } else {
            i++;
        }
    }

    return true;
}

}
