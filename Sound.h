#ifndef SOUNDMANAGER_H
#define SOUNDMANAGER_H

#include <SFML/Audio.hpp>
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>
#include <glm/glm.hpp>


class SoundManager {
public:
    enum SoundType {
        WALK,
        JUMP,
        LAND,
        COUNT,
        FLASHLIGHT,
        BATTERY,
        BORNFIRE,
        FIRE,
        NECK
    };

    SoundManager();
    ~SoundManager();

    bool loadSounds();
    void playSound(SoundType type, float volume = 100.0f, bool loop = false);
    void stopSound(SoundType type);
    void setListenerPosition(float x, float y, float z);
    void setListenerDirection(float x, float y, float z);
    void updateSoundPosition(SoundType type, float x, float y, float z);
    void updateAllSoundPositions(float x, float y, float z);
    void playFireSound(const glm::vec3& position, float volume = 80.0f, bool loop = true);
    void updateFireSoundPositions(const std::vector<glm::vec3>& positions);
    void stopAllFireSounds();

private:
    struct SoundData {
        std::vector<std::unique_ptr<sf::SoundBuffer>> buffers;
        std::vector<std::unique_ptr<sf::Sound>> sounds;
        size_t currentIndex = 0;
    };

    std::unordered_map<SoundType, SoundData> sounds;
    std::unordered_map<SoundType, std::string> soundFiles;

    void loadSoundVariants(SoundType type, const std::string& basePath, int count);
};

#endif