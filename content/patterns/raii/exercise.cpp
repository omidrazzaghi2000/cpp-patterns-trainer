// Exercise: make Transaction an RAII guard so a failed money transfer is rolled back
// automatically - no try/catch inside transfer() allowed.
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
    // TODO 1: acquire the resource - start the transaction with db_.begin().
    explicit Transaction(Database& db) : db_(db) {}

    // TODO 2: release it - if commit() was never called, roll back.
    ~Transaction() {}

    Transaction(const Transaction&) = delete;
    Transaction& operator=(const Transaction&) = delete;

    // TODO 3: commit through the database and remember that it happened.
    void commit() {}

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
