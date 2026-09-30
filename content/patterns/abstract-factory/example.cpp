// Abstract Factory: one factory creates a matching family of database objects.
#include <iostream>
#include <memory>
#include <string>
#include <string_view>

// Abstract products: every database backend must provide both.
class Connection {
public:
    virtual ~Connection() = default;
    virtual void execute(std::string_view sql, int param) const = 0;
};
class SqlDialect {
public:
    virtual ~SqlDialect() = default;
    virtual std::string placeholder(int index) const = 0;  // "$1" or "?"
};

// Family 1: PostgreSQL numbers its parameters: $1, $2, ...
class PgConnection : public Connection {
public:
    void execute(std::string_view sql, int param) const override {
        std::cout << "  [libpq]   " << sql << "   <- " << param << '\n';
    }
};
class PgDialect : public SqlDialect {
public:
    std::string placeholder(int i) const override { return "$" + std::to_string(i); }
};

// Family 2: SQLite uses anonymous ? placeholders.
class SqliteConnection : public Connection {
public:
    void execute(std::string_view sql, int param) const override {
        std::cout << "  [sqlite3] " << sql << "   <- " << param << '\n';
    }
};
class SqliteDialect : public SqlDialect {
public:
    std::string placeholder(int /*index*/) const override { return "?"; }
};

// The abstract factory: one creation method per product in the family.
class DatabaseFactory {
public:
    virtual ~DatabaseFactory() = default;
    virtual std::unique_ptr<Connection> createConnection() const = 0;
    virtual std::unique_ptr<SqlDialect> createDialect() const = 0;
};

// Concrete factories: each one guarantees that its products belong together.
class PostgresFactory : public DatabaseFactory {
public:
    std::unique_ptr<Connection> createConnection() const override {
        return std::make_unique<PgConnection>();
    }
    std::unique_ptr<SqlDialect> createDialect() const override {
        return std::make_unique<PgDialect>();
    }
};

class SqliteFactory : public DatabaseFactory {
public:
    std::unique_ptr<Connection> createConnection() const override {
        return std::make_unique<SqliteConnection>();
    }
    std::unique_ptr<SqlDialect> createDialect() const override {
        return std::make_unique<SqliteDialect>();
    }
};

// Client: knows only abstract types, so it can never mix two families.
void findUser(const DatabaseFactory& db, int id) {
    const auto conn = db.createConnection();
    const auto dialect = db.createDialect();
    conn->execute("SELECT * FROM users WHERE id = " + dialect->placeholder(1), id);
}

// The one place where a concrete family is chosen (e.g. from a config file).
std::unique_ptr<DatabaseFactory> makeFactory(std::string_view env) {
    if (env == "production") return std::make_unique<PostgresFactory>();
    return std::make_unique<SqliteFactory>();
}

int main() {
    for (const std::string_view env : {"production", "unit-test"}) {
        std::cout << env << ":\n";
        findUser(*makeFactory(env), 42);
    }
}
