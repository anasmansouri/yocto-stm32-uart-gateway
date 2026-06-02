#pragma once

#include <atomic>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace cc::manager {

class IpcServer {
public:
    explicit IpcServer(std::string socketPath);
    ~IpcServer();

    bool start();
    void stop();

    bool broadcastLine(const std::string& line);

private:
    void acceptLoop();
    void closeClient(int clientFd);

    std::string socketPath_;

    int serverFd_{-1};

    std::atomic<bool> running_{false};

    std::thread acceptThread_;

    std::mutex clientsMutex_;
    std::vector<int> clients_;
};

}
