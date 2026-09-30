// Object Pool: reuse expensive database connections through RAII handles.
#include <cstddef>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string_view>
#include <vector>

class Connection {
public:
    explicit Connection(int id) : id_(id) {
        std::cout << "  (opening connection #" << id_ << ": slow TLS handshake)\n";
    }
    void query(std::string_view sql) const {
        std::cout << "  #" << id_ << " runs: " << sql << '\n';
    }
    void reset() {}  // real code: roll back open transactions, clear session vars
    int id() const { return id_; }

private:
    int id_;
};

class ConnectionPool {
public:
    // A deleter that gives the object back instead of destroying it.
    struct ReturnToPool {
        ConnectionPool* pool;
        void operator()(Connection* c) const { pool->release(c); }
    };
    using Handle = std::unique_ptr<Connection, ReturnToPool>;

    explicit ConnectionPool(std::size_t maxSize) : maxSize_(maxSize) {
        idle_.reserve(maxSize_);  // release() then never reallocates, so it can't throw
    }
    ConnectionPool(const ConnectionPool&) = delete;  // handles point back to *this
    ConnectionPool& operator=(const ConnectionPool&) = delete;

    Handle acquire() {
        if (idle_.empty()) {  // nothing to reuse: create one, if allowed
            if (all_.size() == maxSize_) throw std::runtime_error("pool exhausted");
            all_.push_back(std::make_unique<Connection>(static_cast<int>(all_.size()) + 1));
            idle_.push_back(all_.back().get());
        }
        Connection* c = idle_.back();
        idle_.pop_back();
        return Handle(c, ReturnToPool{this});
    }
    std::size_t idleCount() const { return idle_.size(); }

private:
    void release(Connection* c) {
        c->reset();  // never hand out an object with leftover state
        idle_.push_back(c);
        std::cout << "  (connection #" << c->id() << " back in the pool)\n";
    }

    std::size_t maxSize_;
    std::vector<std::unique_ptr<Connection>> all_;  // the pool owns every object
    std::vector<Connection*> idle_;                 // the ones ready for reuse
};

int main() {
    ConnectionPool pool(2);  // declared first, so it outlives every handle
    {
        auto a = pool.acquire();
        auto b = pool.acquire();
        a->query("SELECT * FROM orders");
        b->query("UPDATE stock SET qty = qty - 1");
    }  // a and b leave scope: returned to the pool, not destroyed
    std::cout << "idle connections: " << pool.idleCount() << '\n';

    for (int request = 1; request <= 2; ++request) {
        auto conn = pool.acquire();  // reused: no new handshake
        conn->query("SELECT name FROM users");
    }

    auto x = pool.acquire();
    auto y = pool.acquire();
    try {
        auto z = pool.acquire();  // a third one would exceed the limit
    } catch (const std::runtime_error& e) {
        std::cout << "request rejected: " << e.what() << '\n';
    }
}
