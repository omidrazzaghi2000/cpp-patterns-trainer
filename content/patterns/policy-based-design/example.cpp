// Policy-Based Design: a file archiver assembled from policies at compile time.
#include <concepts>
#include <cstddef>
#include <iostream>
#include <string>
#include <string_view>

// C++20 concepts spell out what each kind of policy must provide.
template <typename P>
concept Compressor = requires(const P& p, std::string_view s) {
    { p.compress(s) } -> std::same_as<std::string>;
};
template <typename P>
concept Encryptor = requires(const P& p, std::string s) {
    { p.encrypt(s) } -> std::same_as<std::string>;
};

// ---- Compression policies: plain classes, no base class, no virtual.
struct NoCompression {
    std::string compress(std::string_view data) const { return std::string(data); }
};
struct RunLength {  // "aaab" -> "a3b1"
    std::string compress(std::string_view data) const {
        std::string out;
        for (std::size_t i = 0; i < data.size();) {
            std::size_t run = 1;
            while (i + run < data.size() && data[i + run] == data[i]) ++run;
            out += data[i] + std::to_string(run);
            i += run;
        }
        return out;
    }
};

// ---- Encryption policies (a toy cipher, just to make the effect visible).
struct NoEncryption {
    std::string encrypt(std::string data) const { return data; }
};
class ShiftCipher {
public:
    void setKey(int key) { key_ = key; }  // becomes part of the host's interface!
    std::string encrypt(std::string data) const {
        for (char& c : data)
            if (c >= 'a' && c <= 'z') c = static_cast<char>('a' + (c - 'a' + key_) % 26);
        return data;
    }
private:
    int key_ = 1;
};

// The host: owns the algorithm skeleton, inherits the pluggable details.
// Empty policies add no size thanks to the empty base optimization.
template <Compressor Compress = NoCompression, Encryptor Encrypt = NoEncryption>
class Archiver : public Compress, public Encrypt {
public:
    std::string pack(std::string_view data) const {
        // Both calls are resolved at compile time and can be inlined.
        return this->encrypt(this->compress(data));
    }
};

// Each configuration is just a type alias: no runtime switches.
using PlainArchiver = Archiver<>;
using CompactArchiver = Archiver<RunLength>;
using SecureArchiver = Archiver<RunLength, ShiftCipher>;

int main() {
    const std::string_view data = "aaaabbbccd";
    std::cout << "input:   " << data << '\n';
    std::cout << "plain:   " << PlainArchiver{}.pack(data) << '\n';
    std::cout << "compact: " << CompactArchiver{}.pack(data) << '\n';

    SecureArchiver secure;
    std::cout << "secure:  " << secure.pack(data) << '\n';
    secure.setKey(3);  // only archivers with a ShiftCipher have setKey()
    std::cout << "key 3:   " << secure.pack(data) << '\n';

    std::cout << "size of CompactArchiver: " << sizeof(CompactArchiver)
              << " byte (empty policies are free)\n";
    // Archiver<int> broken;  // error: 'int' does not satisfy 'Compressor'
}
