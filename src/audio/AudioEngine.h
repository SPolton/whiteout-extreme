
#ifndef _AUDIO_ENGINE_H_
#define _AUDIO_ENGINE_H_
// checks if audio engine was defined

#include "fmod_studio.hpp"
#include "fmod.hpp"
#include <string>
#include <map>
#include <vector>
#include <math.h>
#include <iostream>

#include "utils/parser.hpp"

// initailizes and shutsdown FMOD
// will have map of all sounds and events
struct Implementation {
    Implementation();
    ~Implementation();

    void update();

    FMOD::Studio::System* mpStudioSystem;
    FMOD::System* mpSystem;

    int mnNextChannelId;

    typedef std::map<std::string, FMOD::Sound*> SoundMap;
    typedef std::map<int, FMOD::Channel*> ChannelMap;
    typedef std::map<std::string, FMOD::Studio::EventInstance*> EventMap;
    typedef std::map<std::string, FMOD::Studio::Bank*> BankMap;
    typedef parser::json::SoundMap JsonSoundMap;

    BankMap mBanks;
    EventMap mEvents;
    SoundMap mSounds;
    ChannelMap mChannels;
    JsonSoundMap mJsonSounds;
    bool mJsonRegistryLoaded = false;
    bool mJsonRegistryLoadAttempted = false;
    std::string mJsonRegistryPath;
};

// Audio engine
/*
* calls to Implementation struct to start, stop, and update FMOD
* handles basic things like loading, playing, stopping, and updating information on sounds and events
*/
class AudioEngine {
public:
    // TODO: replace with glm::vec3
    struct Vector3 {
        float x;
        float y;
        float z;
    };

    static void init();
    static void update();
    static void shutdown();
    static int errorCheck(FMOD_RESULT result);

    bool loadSoundRegistry(const std::string& manifestPath = "assets/audio/sounds.json");
    void loadSound(const std::string& strSoundName, bool b3d = true, bool bLooping = false, bool bStream = false);
    void unLoadSound(const std::string& strSoundName);

    int playSounds(const std::string& strSoundName, const Vector3& vPos = Vector3{ 0, 0, 0 }, float fVolumeDB = 0.0f);
    int playSoundByName(const std::string& strSoundName, const Vector3& vPos, float fVolumeDB, bool startPaused);
    int jsonSound(const std::string& soundId, bool startPaused = false);
    int jsonSound(const std::string& soundId, const Vector3& vPos, bool startPaused = false);

    void pauseChannel(int nChannelId);
    void resumeChannel(int nChannelId);
    void stopChannel(int nChannelId);
    void setChannel3dPosition(int nChannelId, const Vector3& vPos);
    void set3dListenerAndOrientation(const Vector3& vPos = Vector3{ 0, 0, 0 }, float fVolumeDB = 0.0f);
    void setChannelVolume(int nChannelId, float fVolumeDB);
    float dbToVolume(float dB);
    float volumeToDB(float volume);
    FMOD_VECTOR vectorToFmod(const Vector3& vPos);

};

#endif
