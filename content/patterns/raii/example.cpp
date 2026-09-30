// RAII: a socket wrapper that always closes its handle - even when an exception is thrown.
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>

// A C-style API (think POSIX sockets): whoever opens a handle must remember to close it.
int open_socket(const std::string& host) {
    static int nextFd = 3;
    std::cout << "  open fd " << nextFd << " -> " << host << '\n';
    return nextFd++;
}
void close_socket(int fd) { std::cout << "  close fd " << fd << '\n'; }

// The RAII wrapper: acquire in the constructor, release in the destructor.
// std::unique_ptr (memory) and std::lock_guard (mutexes) follow exactly the same idea.
class Socket {
public:
    explicit Socket(const std::string& host) : fd_(open_socket(host)) {}
    ~Socket() {
        if (fd_ != -1) close_socket(fd_);
    }

    // Exactly one owner: a copy would close the same handle twice.
    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;

    // Moving transfers ownership; the moved-from object is left empty (-1).
    Socket(Socket&& other) noexcept : fd_(std::exchange(other.fd_, -1)) {}
    Socket& operator=(Socket&& other) noexcept {
        if (this != &other) {
            if (fd_ != -1) close_socket(fd_);
            fd_ = std::exchange(other.fd_, -1);
        }
        return *this;
    }

    void send(const std::string& msg) const {
        if (msg.empty()) throw std::runtime_error("refusing to send an empty message");
        std::cout << "  fd " << fd_ << " <- " << msg << '\n';
    }

private:
    int fd_ = -1;
};

void syncInventory(const std::string& payload) {
    Socket sock("inventory.local");
    sock.send("HELLO");
    sock.send(payload);  // may throw - no try/catch and no manual close needed here
    std::cout << "  sync done\n";
}  // ~Socket() runs here: on a normal return AND during stack unwinding

int main() {
    std::cout << "sync #1\n";
    syncInventory("sku=42 qty=7");

    std::cout << "sync #2\n";
    try {
        syncInventory("");  // fails halfway through
    } catch (const std::exception& e) {
        std::cout << "  caught: " << e.what() << '\n';  // the socket is already closed
    }

    std::cout << "hand-over\n";
    Socket a("metrics.local");
    Socket b = std::move(a);  // ownership moves; the handle is still closed exactly once
    std::cout << "end of main\n";
}
