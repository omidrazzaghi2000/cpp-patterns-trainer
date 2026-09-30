// Singleton: one shared application configuration.
#include <iostream>
#include <map>
#include <string>

class AppConfig {
public:
    // The single global access point. Since C++11 a function-local static
    // is initialized exactly once, even if several threads call this at once.
    static AppConfig& instance() {
        static AppConfig config;  // "Meyers' Singleton"
        return config;
    }

    // No copies, no moves: a second AppConfig must never exist.
    AppConfig(const AppConfig&) = delete;
    AppConfig& operator=(const AppConfig&) = delete;
    AppConfig(AppConfig&&) = delete;
    AppConfig& operator=(AppConfig&&) = delete;

    void set(const std::string& key, const std::string& value) { values_[key] = value; }

    std::string get(const std::string& key) const {
        auto it = values_.find(key);
        return it != values_.end() ? it->second : "<unset>";
    }

    int constructions() const { return constructions_; }

private:
    AppConfig() {  // private: only instance() can build the object
        ++constructions_;
        values_["theme"] = "light";
        std::cout << "[AppConfig] defaults loaded\n";
    }

    std::map<std::string, std::string> values_;
    int constructions_ = 0;
};

void renderFrame() {
    // No parameter needed: any module can reach the shared object.
    std::cout << "renderer uses theme: " << AppConfig::instance().get("theme") << '\n';
}

int main() {
    std::cout << "main() starts\n";
    AppConfig::instance().set("theme", "dark");  // first call creates the object
    renderFrame();                               // sees the same object

    AppConfig& a = AppConfig::instance();
    AppConfig& b = AppConfig::instance();
    std::cout << std::boolalpha << "same object: " << (&a == &b) << '\n';
    std::cout << "constructor ran " << a.constructions() << " time(s)\n";
    // AppConfig copy = a;  // compile error: copy constructor is deleted
}
