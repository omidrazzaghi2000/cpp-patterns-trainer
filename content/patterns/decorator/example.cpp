// Decorator: stack compression and encryption onto any data sink at runtime.
#include <cstddef>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

// Component: anything we can write data to.
class DataSink {
public:
    virtual ~DataSink() = default;
    virtual void write(std::string_view data) = 0;
};

// Concrete component: the real destination.
class FileSink : public DataSink {
public:
    explicit FileSink(std::string path) : path_(std::move(path)) {}
    void write(std::string_view data) override {
        std::cout << "  " << path_ << " <- " << data << '\n';
    }

private:
    std::string path_;
};

// Base decorator: IS-A DataSink and HAS-A DataSink. By default it just forwards.
class SinkDecorator : public DataSink {
public:
    explicit SinkDecorator(std::unique_ptr<DataSink> inner) : inner_(std::move(inner)) {}
    void write(std::string_view data) override { inner_->write(data); }

private:
    std::unique_ptr<DataSink> inner_;
};

// Concrete decorator: run-length compression ("aaab" -> "a3b1").
class Compress : public SinkDecorator {
public:
    using SinkDecorator::SinkDecorator;
    void write(std::string_view data) override {
        std::string out;
        for (std::size_t i = 0; i < data.size();) {
            std::size_t run = 1;
            while (i + run < data.size() && data[i + run] == data[i]) ++run;
            out += data[i];
            out += std::to_string(run);
            i += run;
        }
        SinkDecorator::write(out);  // add behavior, then delegate to the wrapped sink
    }
};

// Concrete decorator: toy cipher that shifts every character by one.
class Encrypt : public SinkDecorator {
public:
    using SinkDecorator::SinkDecorator;
    void write(std::string_view data) override {
        std::string out(data);
        for (char& c : out) c = static_cast<char>(c + 1);
        SinkDecorator::write(out);
    }
};

int main() {
    const std::string_view record = "aaaabbbcc";

    std::cout << "plain:\n";
    FileSink("log.txt").write(record);

    // The outermost wrapper runs first: compress, then encrypt, then hit the file.
    std::cout << "compress -> encrypt:\n";
    std::unique_ptr<DataSink> sink = std::make_unique<Compress>(
        std::make_unique<Encrypt>(std::make_unique<FileSink>("backup.bin")));
    sink->write(record);

    // Same classes, different order: the result changes.
    std::cout << "encrypt -> compress:\n";
    sink = std::make_unique<Encrypt>(
        std::make_unique<Compress>(std::make_unique<FileSink>("backup.bin")));
    sink->write(record);
}
