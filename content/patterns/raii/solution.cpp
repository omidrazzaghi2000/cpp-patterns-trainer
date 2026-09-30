// Solution: Transaction begins in its constructor and rolls back in its destructor
// unless commit() ran - the cleanup can no longer be forgotten.
#include <iostream>
#include <stdexcept>
#include <string>

// A fake database connection that only logs what it is asked to do.
class Database {
public:
    void begin() { std::cout << "BEGIN\n"; }
    void commit() { std::cout << "COMMIT\n"; }
    void rollback() { std::cout << "ROLLBACK\n"; }
    void execute(const std::string& stmt) {
        // Simulated failure: another process holds a lock on carol's account.
        if (stmt.starts_with("credit carol")) throw std::runtime_error("carol is locked");
        std::cout << "  " << stmt << '\n';
    }
};

class Transaction {
public:
    explicit Transaction(Database& db) : db_(db) { db_.begin(); }

    ~Transaction() {
        if (!committed_) db_.rollback();
    }

    Transaction(const Transaction&) = delete;
    Transaction& operator=(const Transaction&) = delete;

    void commit() {
        db_.commit();
        committed_ = true;
    }

private:
    Database& db_;
    bool committed_ = false;
};

void transfer(Database& db, const std::string& from, const std::string& to, int amount) {
    Transaction tx(db);
    db.execute("debit " + from + " " + std::to_string(amount));
    db.execute("credit " + to + " " + std::to_string(amount));  // may throw
    tx.commit();
}

int main() {
    Database db;
    transfer(db, "alice", "bob", 100);
    try {
        transfer(db, "alice", "carol", 50);
    } catch (const std::exception& e) {
        std::cout << "error: " << e.what() << '\n';
    }
}
