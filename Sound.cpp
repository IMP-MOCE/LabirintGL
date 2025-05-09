#include "Sound.h"
#include "iostream"
SoundManager::SoundManager() {
    soundFiles[WALK] = "sounds/walk";
    soundFiles[JUMP] = "sounds/jump";
    soundFiles[LAND] = "sounds/land";
    soundFiles[FLASHLIGHT] = "sounds/flashlight";
    soundFiles[BATTERY] = "sounds/battery";
    soundFiles[BORNFIRE] = "sounds/bornfire";
    soundFiles[FIRE] = "sounds/fire";
    soundFiles[NECK] = "sounds/Neck";
}

SoundManager::~SoundManager() {
}

bool SoundManager::loadSounds() {
    loadSoundVariants(WALK, soundFiles[WALK], 1); 
    loadSoundVariants(JUMP, soundFiles[JUMP], 1);
    loadSoundVariants(LAND, soundFiles[LAND], 0);
    loadSoundVariants(FLASHLIGHT, soundFiles[FLASHLIGHT], 1);
    loadSoundVariants(BATTERY, soundFiles[BATTERY], 1);
    loadSoundVariants(BORNFIRE, soundFiles[BORNFIRE], 1);
    loadSoundVariants(FIRE, soundFiles[FIRE], 5);
    loadSoundVariants(NECK, soundFiles[NECK], 1);

    return true;
}

void SoundManager::loadSoundVariants(SoundType type, const std::string& basePath, int count) {
    SoundData data;

    for (int i = 1; i <= count; ++i) {
        std::string path = basePath + std::to_string(i) + ".wav";
        auto buffer = std::make_unique<sf::SoundBuffer>();
        if (buffer->loadFromFile(path)) {
            auto sound = std::make_unique<sf::Sound>();
            sound->setBuffer(*buffer);
            sound->setRelativeToListener(false);

            if (type == FIRE) {
                sound->setMinDistance(20.0f);
                sound->setAttenuation(0.8f);
            }

            data.buffers.push_back(std::move(buffer));
            data.sounds.push_back(std::move(sound));
        }
        else {
            std::cerr << "Failed to load sound: " << path << std::endl;
        }
    }

    sounds[type] = std::move(data);
}

void SoundManager::playSound(SoundType type, float volume, bool loop) {
    auto& soundData = sounds[type];
    if (soundData.sounds.empty()) return;
    size_t index = soundData.currentIndex % soundData.sounds.size();
    soundData.currentIndex++;

    auto& sound = soundData.sounds[index];
    sound->setVolume(volume);
    sound->setLoop(loop);
    sound->play();
}

void SoundManager::stopSound(SoundType type) {
    auto& soundData = sounds[type];
    for (auto& sound : soundData.sounds) {
        sound->stop();
    }
}

void SoundManager::setListenerPosition(float x, float y, float z) {
    sf::Listener::setPosition(x, y, z);
}

void SoundManager::setListenerDirection(float x, float y, float z) {
    sf::Listener::setDirection(x, y, z);
}

void SoundManager::updateSoundPosition(SoundType type, float x, float y, float z) {
    auto it = sounds.find(type);
    if (it != sounds.end()) {
        for (auto& sound : it->second.sounds) {
            sound->setPosition(x, y, z);
        }
    }
}

void SoundManager::updateAllSoundPositions(float x, float y, float z) {
    for (auto& entry : sounds) {
        updateSoundPosition(entry.first, x, y, z);
    }
}

void SoundManager::playFireSound(const glm::vec3& position, float volume, bool loop) {
    auto& fireData = sounds[FIRE];
    if (fireData.sounds.empty()) return;

    // »щем свободный звуковой слот
    for (auto& sound : fireData.sounds) {
        if (sound->getStatus() != sf::Sound::Playing) {
            sound->setPosition(position.x, position.y, position.z);
            sound->setVolume(volume);
            sound->setLoop(loop);
            sound->play();
            return;
        }
    }
}

void SoundManager::updateFireSoundPositions(const std::vector<glm::vec3>& positions) {
    auto& fireData = sounds[FIRE];
    for (size_t i = 0; i < fireData.sounds.size() && i < positions.size(); ++i) {
        fireData.sounds[i]->setPosition(positions[i].x, positions[i].y, positions[i].z);
    }
}

void SoundManager::stopAllFireSounds() {
    auto& fireData = sounds[FIRE];
    for (auto& sound : fireData.sounds) {
        sound->stop();
    }
}