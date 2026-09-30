// Exercise: adapt a third-party syslog client to the app's own Logger interface.
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

// TODO 1: write class SyslogAdapter : public Logger that holds a SyslogClient member
//         (take it in an explicit constructor).
// TODO 2: implement log(): map Info -> 6, Warning -> 4, Error -> 3 and forward the
//         message to SyslogClient::send() (hint: std::string(message)).

// Business code: must not change when the logging backend changes.
void backupDatabase(Logger& logger) {
    logger.log(Level::Info, "backup started");
    logger.log(Level::Warning, "disk 91% full");
    logger.log(Level::Error, "backup failed: no space left");
}

int main() {
    ConsoleLogger logger;  // TODO 3: use SyslogAdapter logger{SyslogClient{"backupd"}};
    backupDatabase(logger);
}
