// Solution: SyslogAdapter translates Logger calls into SyslogClient calls.
#include <iostream>
#include <string>
#include <string_view>
#include <utility>

enum class Level { Info, Warning, Error };

// Target: the interface the whole application logs through.
class Logger {
public:
    virtual ~Logger() = default;
    virtual void log(Level level, std::string_view message) = 0;
};

// The logger we use today. It works, but ops wants everything in syslog.
class ConsoleLogger : public Logger {
public:
    void log(Level level, std::string_view message) override {
        const char* tag = level == Level::Error     ? "ERROR"
                        : level == Level::Warning ? "WARN"
                                                  : "INFO";
        std::cout << '[' << tag << "] " << message << '\n';
    }
};

// Adaptee: a third-party library - do NOT modify it.
// Syslog severities: 3 = error, 4 = warning, 6 = informational.
class SyslogClient {
public:
    explicit SyslogClient(std::string appName) : appName_(std::move(appName)) {}
    void send(int severity, const std::string& text) const {
        std::cout << '<' << severity << "> " << appName_ << ": " << text << '\n';
    }

private:
    std::string appName_;
};

// Adapter: speaks Logger on the outside, SyslogClient on the inside.
class SyslogAdapter : public Logger {
public:
    explicit SyslogAdapter(SyslogClient client) : client_(std::move(client)) {}

    void log(Level level, std::string_view message) override {
        client_.send(toSeverity(level), std::string(message));
    }

private:
    static int toSeverity(Level level) {
        switch (level) {
            case Level::Info: return 6;
            case Level::Warning: return 4;
            case Level::Error: return 3;
        }
        return 6;  // not reached: every Level is handled above
    }

    SyslogClient client_;
};

// Business code: must not change when the logging backend changes.
void backupDatabase(Logger& logger) {
    logger.log(Level::Info, "backup started");
    logger.log(Level::Warning, "disk 91% full");
    logger.log(Level::Error, "backup failed: no space left");
}

int main() {
    SyslogAdapter logger{SyslogClient{"backupd"}};
    backupDatabase(logger);
}
