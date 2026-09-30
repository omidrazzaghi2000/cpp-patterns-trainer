// Null Object: a silent audio service lets game code drop every null check.
#include <iostream>
#include <memory>
#include <string_view>

class AudioService {
public:
    virtual ~AudioService() = default;
    virtual void playSound(std::string_view file) = 0;
    virtual void setVolume(int percent) = 0;
    virtual int volume() const = 0;
};

class SpeakerAudio final : public AudioService {
public:
    void playSound(std::string_view file) override {
        std::cout << "  [speaker] " << file << " at " << volume_ << "%\n";
    }
    void setVolume(int percent) override { volume_ = percent; }
    int volume() const override { return volume_; }

private:
    int volume_ = 80;
};

// The Null Object: a full implementation whose behaviour is "do nothing".
class SilentAudio final : public AudioService {
public:
    void playSound(std::string_view) override {}  // intentionally empty
    void setVolume(int) override {}
    int volume() const override { return 0; }      // a safe, neutral answer
};

// Never returns nullptr: "no device" is represented by a real object.
std::unique_ptr<AudioService> openAudio(bool deviceFound) {
    if (deviceFound) return std::make_unique<SpeakerAudio>();
    return std::make_unique<SilentAudio>();
}

class Game {
public:
    explicit Game(AudioService& audio) : audio_(audio) {}

    // No null checks anywhere: whatever device we got, every call is safe.
    void run() {
        audio_.setVolume(50);
        std::cout << "player jumps\n";
        audio_.playSound("jump.wav");
        std::cout << "coin collected\n";
        audio_.playSound("coin.wav");
        std::cout << "HUD volume: " << audio_.volume() << "%\n";
    }

private:
    AudioService& audio_;  // a reference: it can't be null, by design
};

int main() {
    std::cout << "== gaming laptop ==\n";
    auto speakers = openAudio(true);
    Game(*speakers).run();

    std::cout << "== headless test server (no sound card) ==\n";
    auto silence = openAudio(false);
    Game(*silence).run();  // exactly the same game code, just quiet
}
